#include <Arduino.h>
#include <controllers/ESCControllers.h>
#include <network/WebServerManager.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <devices/QMC5883L.h>
#include <inputs/AnalogInput.h>
#include <devices/battery/BatteryMonitor.h>
#include <devices/currentMeasure/CurrentMeasure.h>
#include <BMP280/BMP280Sensor.h>
#include <orientation/SimpleMahony.h>
// === Cấu hình ===
#define STATUS_LED_PIN 2

// Định nghĩa tần số lấy mẫu
#define SAMPLE_FREQ 100  // 100Hz
#define SAMPLE_TIME (1000000/SAMPLE_FREQ) // Microseconds

const char *ssid = "ESP32-WIFI";
const char *password = "12345678";

// === Khai báo đối tượng ===
ESCController escController;
StatusLED statusLed(STATUS_LED_PIN);
MPU6500 mpu(5);
QMC5883L compass;
AnalogInput batteryInput(33, 4.066, 0.0, 1.0, 0.1);
AnalogInput currentInput(34, 1.99686, 0.0, 1.0, 0.05);
BatteryMonitor battery(&batteryInput);
CurrentMonitor current(&currentInput);
BME280Sensor bme280;
SimpleMahony mahony(SAMPLE_FREQ);
WebServerManager webServer(ssid, password, escController, statusLed, mpu, battery, current, bme280);

// === Khai báo TaskHandle ===
TaskHandle_t sensorTaskHandle;
TaskHandle_t webTaskHandle;

// Hàm quét I2C
void scanI2C() {
    Serial.println("\nQuét I2C bus:");
    for(uint8_t addr = 0; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        if(error == 0) {
            Serial.printf("Tìm thấy thiết bị tại địa chỉ 0x%02X\n", addr);
        }
    }
    Serial.println("Quét I2C hoàn tất\n");
}

// === Task: Cảm biến (core 1) ===
void sensorTask(void *parameter)
{
  Serial.println("\n=== Khởi tạo cảm biến ===");
  
  // Khởi tạo I2C
  Wire.begin();
  delay(100);
  
  // Quét I2C trước khi khởi tạo cảm biến
  scanI2C();
  
  // Khởi tạo MPU6500
  Serial.println("\nKhởi tạo MPU6500...");
  mpu.begin();  // Bỏ kiểm tra return vì hàm begin() trả về void
  Serial.println("MPU6500 đã sẵn sàng");
  
  // Test đọc MPU6500
  Serial.println("\nTest MPU6500:");
  for(int i = 0; i < 5; i++) {
    mpu.readSensor();
    Serial.printf("Accel: X=%.2f Y=%.2f Z=%.2f | Gyro: X=%.2f Y=%.2f Z=%.2f\n",
                 mpu.getAccelX(), mpu.getAccelY(), mpu.getAccelZ(),
                 mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ());
    delay(100);
  }
  
  // Khởi tạo QMC5883L
  Serial.println("\nKhởi tạo QMC5883L...");
  if (!compass.begin()) {
    Serial.println("Lỗi khởi tạo QMC5883L!");
    while(1) {
      statusLed.blink(100, 3);
      vTaskDelay(1000);
    }
  }
  Serial.println("QMC5883L đã sẵn sàng");
  
  // Test đọc QMC5883L
  Serial.println("\nTest QMC5883L:");
  float mx, my, mz;  // Khai báo biến cho test
  for(int i = 0; i < 5; i++) {
    compass.readMag(mx, my, mz);
    Serial.printf("Mag: X=%.2f Y=%.2f Z=%.2f\n", mx, my, mz);
    delay(100);
  }
  
  batteryInput.begin();
  currentInput.begin();

  if (!bme280.begin()) {
    Serial.println("Không tìm thấy BME280!");
  } else {
    Serial.println("BME280 đã sẵn sàng.");
  }

  // Khởi tạo đối tượng Mahony
  mahony.setGains(5.0f, 0.0f, 1.0f); // Kp = 5.0, Ki = 0.0, Km = 1.0

  // Biến lưu thời gian
  unsigned long lastTime = micros();
  unsigned long lastUpdate = millis();
  unsigned long lastPrint = millis(); // Thời gian in log gần nhất
  
  // Chuyển đổi dữ liệu gyro từ độ/giây sang radian/giây
  float gx, gy, gz;
  float ax, ay, az;
  // Sử dụng lại biến mx, my, mz đã khai báo ở trên
  float roll, pitch, yaw;
  
  while (true)
  {
    // Đợi đến chu kỳ lấy mẫu tiếp theo
    if (micros() - lastTime >= SAMPLE_TIME) {
        lastTime = micros();

        // Đọc dữ liệu từ MPU6500
        mpu.readSensor();
        
        // Chuyển đổi dữ liệu gyro từ độ/giây sang radian/giây
        gx = mpu.getGyroX() * DEG_TO_RAD;
        gy = mpu.getGyroY() * DEG_TO_RAD;
        gz = mpu.getGyroZ() * DEG_TO_RAD;
        
        // Dữ liệu accelerometer đã ở đơn vị g
        ax = mpu.getAccelX();
        ay = mpu.getAccelY();
        az = mpu.getAccelZ();

        // Đọc dữ liệu từ QMC5883L
        compass.readMag(mx, my, mz);
        
        // Cập nhật thuật toán Mahony với dữ liệu từ trường
        mahony.update(gx, gy, gz, ax, ay, az, mx, my, mz);
        roll = mahony.getRollDegrees();
        pitch = mahony.getPitchDegrees();
        yaw = mahony.getYawDegrees();

        // === In log mỗi 1000ms (1s) ===
        if (millis() - lastPrint >= 1000) {
            lastPrint = millis();
            Serial.printf("Roll: %.2f, Pitch: %.2f, Yaw: %.2f\n", roll, pitch, yaw);
            Serial.printf("Accel: X=%.2f Y=%.2f Z=%.2f\n", ax, ay, az);
            Serial.printf("Gyro: X=%.2f Y=%.2f Z=%.2f\n", gx, gy, gz);
            Serial.printf("Mag: X=%.2f Y=%.2f Z=%.2f\n", mx, my, mz);
        }
    }

    vTaskDelay(1);
  }
}

// === Task: Web Server (core 0) ===
void webTask(void *parameter)
{
  webServer.begin();
  while (true)
  {
    webServer.handleClient();
    vTaskDelay(1);
  }
}

// === Setup ===
void setup()
{
  Serial.begin(115200);
  delay(1000); // Đợi Serial ổn định
  
  Serial.println("\n=== Khởi động hệ thống ===");
  
  statusLed.begin();
  statusLed.blink(200, 3);
  escController.begin();

  // Tạo task cho cảm biến trên core 1
  xTaskCreatePinnedToCore(
      sensorTask,
      "SensorTask",
      10000,
      NULL,
      1,
      &sensorTaskHandle,
      1);

  // Tạo task cho web server trên core 0
  xTaskCreatePinnedToCore(
      webTask,
      "WebTask",
      10000,
      NULL,
      1,
      &webTaskHandle,
      0);

  statusLed.on(); // Báo hiệu khởi động hoàn tất
}

// === Loop chính (không dùng) ===
void loop()
{
  vTaskDelay(portMAX_DELAY);
}

