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
#include <controllers/DronePIDController.h>
#include <receiver/IBusReceiver.h>
#include <debug/SerialDebug.h>
#include <debug/WebSocketDebug.h>

// === Cấu hình ===
#define STATUS_LED_PIN 2
#define IBUS_RX_PIN 16 // Chân RX cho IBus receiver

// Định nghĩa tần số lấy mẫu và điều khiển
#define SAMPLE_FREQ 500                     // Tăng lên 500Hz cho PID loop
#define SAMPLE_TIME (1000000 / SAMPLE_FREQ) // Microseconds

// Giới hạn góc và PID gains
#define MAX_ANGLE 30.0f     // Góc tối đa cho roll/pitch (độ)
#define MAX_YAW_RATE 200.0f // Tốc độ yaw tối đa (độ/giây)

// PID gains - cần tune lại cho drone của bạn
#define ROLL_KP 4.0f
#define ROLL_KI 0.1f
#define ROLL_KD 0.2f

#define PITCH_KP 4.0f
#define PITCH_KI 0.1f
#define PITCH_KD 0.2f

#define YAW_KP 3.0f
#define YAW_KI 0.1f
#define YAW_KD 0.1f

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
HardwareSerial IBusSerial(2); // Sử dụng UART2 cho IBus
IBusReceiver receiver(&IBusSerial);
DronePIDController pidController(
    ROLL_KP, ROLL_KI, ROLL_KD,
    PITCH_KP, PITCH_KI, PITCH_KD,
    YAW_KP, YAW_KI, YAW_KD,
    100.0f,   // max output
    MAX_ANGLE // max angle
);
WebServerManager webServer(ssid, password, escController, statusLed, mpu, battery, current, bme280, mahony, pidController, receiver);

// === Khai báo TaskHandle ===
TaskHandle_t sensorTaskHandle;
TaskHandle_t webTaskHandle;

// Thêm SerialDebug vào danh sách đối tượng
SerialDebug serialDebug;

// Thêm WebSocketDebug vào danh sách đối tượng
WebSocketDebug webSocketDebug;

// Hàm quét I2C
void scanI2C()
{
    Serial.println("\nQuét I2C bus:");
    for (uint8_t addr = 0; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.printf("Tìm thấy thiết bị tại địa chỉ 0x%02X\n", addr);
        }
    }
    Serial.println("Quét I2C hoàn tất\n");
}

// === Task: Cảm biến và điều khiển (core 1) ===
void sensorTask(void *parameter)
{
    Serial.println("\n=== Khởi tạo cảm biến và điều khiển ===");

    // Khởi tạo I2C
    Wire.begin();
    delay(100);

    // Khởi tạo IBus receiver
    IBusSerial.begin(115200, SERIAL_8N1, IBusSerial, -1); // RX only
    receiver.begin();

    // Khởi tạo các cảm biến
    mpu.begin();
    if (!compass.begin())
    {
        Serial.println("Lỗi khởi tạo QMC5883L!");
        while (1)
        {
            statusLed.blink(100, 3);
            vTaskDelay(1000);
        }
    }

    batteryInput.begin();
    currentInput.begin();
    if (!bme280.begin())
    {
        Serial.println("Không tìm thấy BME280!");
    }

    // Khởi tạo đối tượng Mahony
    mahony.setGains(5.0f, 0.0f, 1.0f);

    // Biến lưu thời gian
    unsigned long lastTime = micros();
    unsigned long lastPrint = millis();

    // Biến cho điều khiển
    float throttle = 0;
    float rollSetpoint = 0, pitchSetpoint = 0, yawSetpoint = 0;
    float rollActual = 0, pitchActual = 0, yawActual = 0;

    unsigned long lastQMCRead = 0;
    const unsigned long QMC_INTERVAL = 10; // ms, tương đương 100Hz

    while (true)
    {
        unsigned long currentTime = micros();
        float dt = (currentTime - lastTime) / 1000000.0f;
        if (dt >= (1.0f / SAMPLE_FREQ))
        {
            lastTime = currentTime;

            // 1. Đọc dữ liệu từ cảm biến
            mpu.readSensor();
            float ax = mpu.getAccelX();
            float ay = mpu.getAccelY();
            float az = mpu.getAccelZ();
            float gx = mpu.getGyroX() * DEG_TO_RAD;
            float gy = mpu.getGyroY() * DEG_TO_RAD;
            float gz = mpu.getGyroZ() * DEG_TO_RAD;

            float mx, my, mz;

            unsigned long now = millis();
            if (now - lastQMCRead >= QMC_INTERVAL)
            {
                lastQMCRead = now;
                if (compass.dataReady())
                {
                    float mx, my, mz;
                    compass.readMag(mx, my, mz);
                    // Xử lý dữ liệu compass ở đây
                }
            }

            // 2. Cập nhật orientation
            mahony.update(gx, gy, gz, ax, ay, az, mx, my, mz);
            rollActual = mahony.getRollDegrees();
            pitchActual = mahony.getPitchDegrees();
            yawActual = mahony.getYawDegrees();

            // Gửi dữ liệu orientation nếu đang ở chế độ config
            serialDebug.sendOrientationData(rollActual, pitchActual, yawActual);

            // 3. Đọc lệnh từ receiver
            if (receiver.read_channel_data())
            {
                // Map receiver values to control ranges
                throttle = map(receiver.get_throttle(), 1000, 2000, 0, 100);
                rollSetpoint = map(receiver.get_roll(), 1000, 2000, -MAX_ANGLE, MAX_ANGLE);
                pitchSetpoint = map(receiver.get_pitch(), 1000, 2000, -MAX_ANGLE, MAX_ANGLE);
                yawSetpoint = map(receiver.get_yaw(), 1000, 2000, -MAX_YAW_RATE, MAX_YAW_RATE);

                // Gửi dữ liệu receiver nếu đang ở chế độ config
                serialDebug.sendReceiverData(
                    receiver.get_throttle(),
                    receiver.get_roll(),
                    receiver.get_pitch(),
                    receiver.get_yaw(),
                    receiver.get_aux1(),
                    receiver.get_aux2());
            }

            // 4. Cập nhật PID controllers
            pidController.update(
                rollSetpoint, pitchSetpoint, yawSetpoint,
                rollActual, pitchActual, yawActual,
                dt);

            // 5. Mix và điều khiển motor
            escController.mixAndSetMotors(
                throttle,
                pidController.getRollOutput(),
                pidController.getPitchOutput(),
                pidController.getYawOutput());

            // Gửi dữ liệu operator mode
            serialDebug.sendOperatorData(
                throttle,
                rollActual, pitchActual, yawActual,
                pidController.getRollOutput(),
                pidController.getPitchOutput(),
                pidController.getYawOutput(),
                rollSetpoint, pitchSetpoint, yawSetpoint,
                escController.getCurrentValue(ESC_FL),
                escController.getCurrentValue(ESC_FR),
                escController.getCurrentValue(ESC_RL),
                escController.getCurrentValue(ESC_RR));

            // Gửi dữ liệu qua WebSocket cho biểu đồ
            webSocketDebug.sendDebugData(
                rollActual, pitchActual, yawActual,
                pidController.getRollOutput(),
                pidController.getPitchOutput(),
                pidController.getYawOutput(),
                rollSetpoint, pitchSetpoint, yawSetpoint,
                escController.getCurrentValue(ESC_FL),
                escController.getCurrentValue(ESC_FR),
                escController.getCurrentValue(ESC_RL),
                escController.getCurrentValue(ESC_RR),
                battery.getVoltage(),
                current.getCurrent(),
                ax, ay, az,
                gx * RAD_TO_DEG, gy * RAD_TO_DEG, gz * RAD_TO_DEG);

            // In debug info mỗi 1 giây
            if (millis() - lastPrint >= 1000)
            {
                lastPrint = millis();
                Serial.printf("Setpoints: R=%.1f P=%.1f Y=%.1f T=%.1f\n",
                              rollSetpoint, pitchSetpoint, yawSetpoint, throttle);
                Serial.printf("Actual: R=%.1f P=%.1f Y=%.1f\n",
                              rollActual, pitchActual, yawActual);
                Serial.printf("PID Output: R=%.1f P=%.1f Y=%.1f\n",
                              pidController.getRollOutput(),
                              pidController.getPitchOutput(),
                              pidController.getYawOutput());
            }
        }

        // Kiểm tra lệnh từ serial
        if (Serial.available())
        {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();
            serialDebug.handleCommand(cmd);
        }

        vTaskDelay(1);
    }
}

// === Task: Web Server (core 0) ===
void webTask(void *parameter)
{
    webServer.begin();
    webSocketDebug.begin(); // Khởi tạo WebSocket
    while (true)
    {
        webServer.handleClient();
        webSocketDebug.handle(); // Xử lý WebSocket
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
