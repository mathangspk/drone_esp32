#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <controllers/ESCControllers.h>
#include <network/WebServerManager.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <devices/QMC5883L.h>
#include <inputs/AnalogInput.h>
#include <devices/battery/BatteryMonitor.h>
#include <devices/currentMeasure/CurrentMeasure.h>
#include <BME280/BME280Sensor.h>
#include <orientation/SimpleMahony.h>
#include <controllers/DronePIDController.h>
#include "receiver/IBusReceiver.h"
#include <debug/SerialDebug.h>
#include <debug/WebSocketDebug.h>
#include <devices/I2CScanner.h>
#include <devices/SensorCalibration.h>

// === Cấu hình ===
#define STATUS_LED_PIN 2
#define IBUS_RX_PIN 16 // Chân RX cho IBus receiver

// Định nghĩa tần số lấy mẫu và điều khiển
#define SAMPLE_FREQ 1000.0f  // 1kHz thay vì 100Hz
#define SAMPLE_TIME (1000000 / SAMPLE_FREQ) // Microseconds

// Giới hạn góc và PID gains
#define MAX_ANGLE 30.0f     // Góc tối đa cho roll/pitch (độ)
#define MAX_YAW_RATE 180.0f // Tốc độ yaw tối đa (độ/giây)

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

const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPassword";

// === Khai báo đối tượng ===
ESCController escController;
StatusLED statusLed(STATUS_LED_PIN);
MPU6500 mpu(5);  // CS pin = 5
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

// Thêm I2CScanner vào danh sách đối tượng
I2CScanner i2cScanner;

// Thêm SensorCalibration vào danh sách đối tượng
SensorCalibration sensorCal;

// === Task: Cảm biến và điều khiển (core 1) ===
void sensorTask(void *parameter)
{
    Serial.println("\n=== Khởi tạo cảm biến và điều khiển ===");

    // Khởi tạo I2C
    Wire.begin();
    delay(100);

    // Thiết lập callbacks cho việc khởi tạo thiết bị I2C
    // MPU6500 kết nối qua SPI nên không cần callback I2C
    i2cScanner.setDeviceCallback(DEVICE_BME280, [&]() -> bool {
        return bme280.begin();
    });
    
    i2cScanner.setDeviceCallback(DEVICE_QMC5883L, [&]() -> bool {
        return compass.begin();
    });

    // Thiết lập error handler
    i2cScanner.setErrorHandler([](DeviceType deviceType, const char* error) {
        Serial.printf("Lỗi thiết bị %d: %s\n", deviceType, error);
    });

    // Thiết lập success handler
    i2cScanner.setSuccessHandler([](DeviceType deviceType, const char* message) {
        Serial.printf("Thành công thiết bị %d: %s\n", deviceType, message);
    });

    // Quét I2C bus và kiểm tra thiết bị
    i2cScanner.scanAndPrint();
    
    // Kiểm tra thiết bị cần thiết
    if (!i2cScanner.checkRequiredDevices()) {
        Serial.println("Cảnh báo: Một số thiết bị cần thiết không được tìm thấy!");
    }

    // Khởi tạo IBus receiver
    IBusSerial.begin(115200, SERIAL_8N1, IBusSerial, -1); // RX only
    receiver.begin();

    // Khởi tạo các thiết bị khác
    batteryInput.begin();
    currentInput.begin();

    // Khởi tạo thiết bị I2C tự động
    i2cScanner.initializeDevices();

    // Khởi tạo MPU6500 qua SPI (không qua I2C Scanner)
    mpu.begin(); // Hàm này trả về void, không cần kiểm tra
    
    // Kiểm tra MPU6500 có hoạt động không
    if (!mpu.isConnected()) {
        Serial.println("Lỗi: MPU6500 (SPI) không hoạt động!");
        while (1) {
            statusLed.blink(100, 3);
            vTaskDelay(1000);
        }
    } else {
        Serial.println("✓ MPU6500 (SPI): Khởi tạo thành công");
    }

    // Kiểm tra thiết bị I2C quan trọng
    if (!i2cScanner.isDeviceWorking(DEVICE_QMC5883L)) {
        Serial.println("Cảnh báo: QMC5883L không được tìm thấy!");
    } else {
        Serial.println("✓ QMC5883L (I2C): Được tìm thấy");
    }

    if (!i2cScanner.isDeviceWorking(DEVICE_BME280)) {
        Serial.println("Cảnh báo: BME280 không được tìm thấy!");
    } else {
        Serial.println("✓ BME280 (I2C): Được tìm thấy");
    }

    // In trạng thái thiết bị
    i2cScanner.printDeviceStatus();

    // Khởi tạo đối tượng Mahony với gains tối ưu theo Betaflight
    mahony.setGains(5.0f, 0.1f, 1.0f);  // Tăng Kp và thêm Ki

    // Thiết lập filter parameters
    sensorCal.setGyroFilter(0.1f);    // Low-pass filter cho gyro
    sensorCal.setAccelFilter(0.1f);   // Low-pass filter cho accel
    sensorCal.setMagFilter(0.1f);     // Low-pass filter cho mag

    // Biến lưu thời gian
    unsigned long lastTime = micros();
    unsigned long lastPrint = millis();
    unsigned long lastSensorDebug = millis();

    // Biến cho điều khiển
    float throttle = 0;
    float rollSetpoint = 0, pitchSetpoint = 0, yawSetpoint = 0;
    float rollActual = 0, pitchActual = 0, yawActual = 0;

    unsigned long lastQMCRead = 0;
    const unsigned long QMC_INTERVAL = 10; // ms, tương đương 100Hz

    // Debug: In thông tin khởi tạo
    Serial.println("=== Khởi tạo hoàn tất ===");
    Serial.println("Mahony filter gains: Kp=5.0, Ki=0.1, Kd=1.0");
    Serial.println("Sample rate: 1kHz (Betaflight-style)");
    Serial.println("Bắt đầu vòng lặp cảm biến...");
    
    // Calibration test
    Serial.println("=== Calibration Test ===");
    Serial.println("Đặt drone nằm phẳng và đứng yên trong 3 giây...");
    delay(3000);
    
    // Đọc và in giá trị sensor ban đầu
    mpu.readSensor();
    Serial.printf("Initial Accel: ax=%.3f ay=%.3f az=%.3f\n", 
                  mpu.getAccelX(), mpu.getAccelY(), mpu.getAccelZ());
    Serial.printf("Initial Gyro: gx=%.3f gy=%.3f gz=%.3f\n", 
                  mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ());
    
    // Reset Mahony filter để bắt đầu clean
    mahony.reset();
    Serial.println("Mahony filter đã reset - bắt đầu tracking...");

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

            // Khởi tạo giá trị magnetometer mặc định
            float mx = 0.0f, my = 0.0f, mz = 0.0f;

            // Debug: Kiểm tra dữ liệu sensor mỗi 5 giây
            if (millis() - lastSensorDebug >= 5000) {
                lastSensorDebug = millis();
                Serial.printf("Sensor Debug - Accel: ax=%.3f ay=%.3f az=%.3f\n", ax, ay, az);
                Serial.printf("Sensor Debug - Gyro: gx=%.3f gy=%.3f gz=%.3f\n", gx, gy, gz);
                Serial.printf("Sensor Debug - Mag: mx=%.3f my=%.3f mz=%.3f\n", mx, my, mz);
                
                // Kiểm tra dữ liệu hợp lệ
                if (isnan(ax) || isnan(ay) || isnan(az) || isnan(gx) || isnan(gy) || isnan(gz)) {
                    Serial.println("LỖI: Dữ liệu sensor có giá trị NaN!");
                }
                
                // Kiểm tra magnitude của accelerometer (nên gần 1g = 9.81 m/s²)
                float accelMagnitude = sqrt(ax*ax + ay*ay + az*az);
                Serial.printf("Accel Magnitude: %.3f (should be ~9.81)\n", accelMagnitude);
                
                // Kiểm tra gyro bias (nên gần 0 khi đứng yên)
                Serial.printf("Gyro Bias Check - gx=%.3f gy=%.3f gz=%.3f\n", gx, gy, gz);
            }

            // Kiểm tra và xử lý dữ liệu NaN
            if (isnan(ax) || isnan(ay) || isnan(az)) {
                ax = 0.0f; ay = 0.0f; az = 1.0f; // Giá trị mặc định
            }
            if (isnan(gx) || isnan(gy) || isnan(gz)) {
                gx = 0.0f; gy = 0.0f; gz = 0.0f; // Giá trị mặc định
            }

            // 2. Áp dụng calibration và filtering (Betaflight-style)
            sensorCal.processGyro(gx, gy, gz);
            sensorCal.processAccel(ax, ay, az);

            unsigned long now = millis();
            if (now - lastQMCRead >= QMC_INTERVAL)
            {
                lastQMCRead = now;
                if (compass.dataReady())
                {
                    compass.readMag(mx, my, mz);
                    sensorCal.processMag(mx, my, mz);
                }
            }

            // 3. Cập nhật orientation
            mahony.update(gx, gy, gz, ax, ay, az, mx, my, mz);
            rollActual = mahony.getRollDegrees();
            pitchActual = mahony.getPitchDegrees();
            yawActual = mahony.getYawDegrees();

            // Debug: Kiểm tra orientation mỗi 5 giây (sử dụng biến khác)
            static unsigned long lastOrientationDebug = 0;
            if (millis() - lastOrientationDebug >= 5000) {
                lastOrientationDebug = millis();
                Serial.printf("Orientation Debug - Roll=%.3f Pitch=%.3f Yaw=%.3f\n", 
                              rollActual, pitchActual, yawActual);
                
                // Kiểm tra giá trị NaN
                if (isnan(rollActual) || isnan(pitchActual) || isnan(yawActual)) {
                    Serial.println("LỖI: Orientation có giá trị NaN!");
                    // Reset Mahony filter nếu cần
                    mahony.reset();
                }
                
                // Test: Kiểm tra xem drone có nằm phẳng không
                if (abs(rollActual) < 1.0 && abs(pitchActual) < 1.0) {
                    Serial.println("✓ Drone đang nằm phẳng (roll và pitch < 1°)");
                } else {
                    Serial.println("⚠ Drone không nằm phẳng - cần calibration hoặc điều chỉnh mounting");
                }
                
                // Test: Kiểm tra yaw có ổn định không
                static float lastYaw = 0;
                float yawDiff = abs(yawActual - lastYaw);
                if (yawDiff < 0.1) {
                    Serial.println("✓ Yaw ổn định");
                } else {
                    Serial.printf("⚠ Yaw thay đổi: %.3f°\n", yawDiff);
                }
                lastYaw = yawActual;
            }

            // Gửi dữ liệu orientation nếu đang ở chế độ config
            serialDebug.sendOrientationData(rollActual, pitchActual, yawActual);

            // 4. Đọc lệnh từ receiver
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

            // 5. Cập nhật PID controllers
            pidController.update(
                rollSetpoint, pitchSetpoint, yawSetpoint,
                rollActual, pitchActual, yawActual,
                dt);

            // 6. Mix và điều khiển motor
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
                Serial.printf("Setpoints: R=%.1f° P=%.1f° Y=%.1f° T=%.1f\n",
                              rollSetpoint, pitchSetpoint, yawSetpoint, throttle);
                Serial.printf("Actual: R=%.1f° P=%.1f° Y=%.1f°\n",
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
            
            // Handle calibration commands
            if (cmd == "CALIBRATE_GYRO") {
                sensorCal.startGyroCalibration();
                // Collect samples for 10 seconds
                unsigned long startTime = millis();
                while (millis() - startTime < 10000) {
                    mpu.readSensor();
                    float gx = mpu.getGyroX() * DEG_TO_RAD;
                    float gy = mpu.getGyroY() * DEG_TO_RAD;
                    float gz = mpu.getGyroZ() * DEG_TO_RAD;
                    sensorCal.addGyroSample(gx, gy, gz);
                    delay(10);
                }
                sensorCal.finishGyroCalibration();
            }
            else if (cmd == "CALIBRATE_ACCEL") {
                sensorCal.startAccelCalibration();
                // Collect samples for 5 seconds
                unsigned long startTime = millis();
                while (millis() - startTime < 5000) {
                    mpu.readSensor();
                    float ax = mpu.getAccelX();
                    float ay = mpu.getAccelY();
                    float az = mpu.getAccelZ();
                    sensorCal.addAccelSample(ax, ay, az);
                    delay(10);
                }
                sensorCal.finishAccelCalibration();
            }
            else if (cmd == "CALIBRATE_MAG") {
                sensorCal.startMagCalibration();
                // Collect samples for 15 seconds while rotating
                unsigned long startTime = millis();
                while (millis() - startTime < 15000) {
                    if (compass.dataReady()) {
                        float mx, my, mz;
                        compass.readMag(mx, my, mz);
                        sensorCal.addMagSample(mx, my, mz);
                    }
                    delay(10);
                }
                sensorCal.finishMagCalibration();
            }
            else if (cmd == "SET_FILTER_GAINS") {
                Serial.println("Nhập Kp (default 5.0):");
                // Simplified - just set default values
                mahony.setGains(5.0f, 0.1f, 1.0f);
                Serial.println("Filter gains đã set: Kp=5.0, Ki=0.1, Kd=1.0");
            }
            else if (cmd == "RESET_ALL") {
                sensorCal.resetAll();
                mahony.reset();
                Serial.println("Tất cả calibrations và filter đã reset!");
            }
            else if (cmd == "RESET_MAHONY") {
                mahony.reset();
                Serial.println("Mahony filter đã reset!");
            }
            else if (cmd == "CALIBRATE") {
                Serial.println("=== Bắt đầu calibration ===");
                Serial.println("Đặt drone nằm phẳng và đứng yên...");
                delay(2000);
                
                // Reset và đọc sensor
                mahony.reset();
                mpu.readSensor();
                Serial.printf("Calibration Accel: ax=%.3f ay=%.3f az=%.3f\n", 
                              mpu.getAccelX(), mpu.getAccelY(), mpu.getAccelZ());
                Serial.printf("Calibration Gyro: gx=%.3f gy=%.3f gz=%.3f\n", 
                              mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ());
                Serial.println("Calibration hoàn tất!");
            }
            else {
                serialDebug.handleCommand(cmd);
            }
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
