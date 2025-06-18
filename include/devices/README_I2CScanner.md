# I2C Scanner Library - Enhanced Version

Thư viện I2C Scanner được thiết kế để quét, phát hiện, kiểm tra và khởi tạo các thiết bị I2C một cách chuyên nghiệp và tự động.

## 🚀 Tính năng mới

- ✅ **Quét toàn bộ bus I2C** (địa chỉ 0-127)
- ✅ **Phát hiện và liệt kê** tất cả thiết bị
- ✅ **Nhận diện 30+ thiết bị** phổ biến (MPU6500, BME280, HMC5883L, v.v.)
- ✅ **Kiểm tra thiết bị cần thiết** (Required vs Optional)
- ✅ **Khởi tạo thiết bị tự động** với callback system
- ✅ **Error handling** và success handling
- ✅ **Trạng thái thiết bị** real-time
- ✅ **API linh hoạt** và dễ sử dụng

## 📋 Cách sử dụng

### 1. Khởi tạo cơ bản

```cpp
#include <devices/I2CScanner.h>

I2CScanner i2cScanner;

void setup() {
    Wire.begin();
    
    // Quét và in kết quả
    i2cScanner.scanAndPrint();
}
```

### 2. Kiểm tra thiết bị cần thiết

```cpp
// Kiểm tra thiết bị cần thiết
if (!i2cScanner.checkRequiredDevices()) {
    Serial.println("Cảnh báo: Một số thiết bị cần thiết không được tìm thấy!");
}

// Kiểm tra thiết bị cụ thể
if (i2cScanner.isDeviceWorking(DEVICE_MPU6500)) {
    Serial.println("MPU6500 hoạt động tốt!");
}
```

### 3. Khởi tạo thiết bị tự động

```cpp
// Thiết lập callbacks cho việc khởi tạo
i2cScanner.setDeviceCallback(DEVICE_MPU6500, [&]() -> bool {
    return mpu.begin();
});

i2cScanner.setDeviceCallback(DEVICE_BME280, [&]() -> bool {
    return bme280.begin();
});

// Khởi tạo tất cả thiết bị
i2cScanner.initializeDevices();
```

### 4. Error và Success Handling

```cpp
// Thiết lập error handler
i2cScanner.setErrorHandler([](DeviceType deviceType, const char* error) {
    Serial.printf("Lỗi thiết bị %d: %s\n", deviceType, error);
});

// Thiết lập success handler
i2cScanner.setSuccessHandler([](DeviceType deviceType, const char* message) {
    Serial.printf("Thành công thiết bị %d: %s\n", deviceType, message);
});
```

### 5. Kiểm tra trạng thái

```cpp
// In trạng thái tất cả thiết bị
i2cScanner.printDeviceStatus();

// Kiểm tra trạng thái cụ thể
if (i2cScanner.isDeviceInitialized(DEVICE_MPU6500)) {
    Serial.println("MPU6500 đã được khởi tạo!");
}

// Lấy địa chỉ thiết bị
uint8_t mpuAddress = i2cScanner.getDeviceAddress(DEVICE_MPU6500);
```

## 🔧 Thiết bị được hỗ trợ

### Thiết bị I2C (được quản lý bởi I2CScanner):
- **BME280** (0x76, 0x77) - Cảm biến nhiệt độ, áp suất, độ ẩm
- **QMC5883L** (0x0D) - Cảm biến từ trường 3 trục
- **HMC5883L** (0x1E) - Cảm biến từ trường 3 trục (backup)

### Thiết bị SPI (không được quản lý bởi I2CScanner):
- **MPU6500** - Cảm biến chuyển động 6 trục (kết nối SPI)

### Thiết bị phổ biến khác:
- **ADS1115** (0x48-0x4B) - ADC 16-bit
- **SSD1306** (0x3C, 0x3D) - Màn hình OLED
- **24CXX EEPROM** (0x50-0x57) - Bộ nhớ EEPROM
- **PCA9685** (0x70-0x77) - Bộ điều khiển PWM 16 kênh
- **MCP23017** (0x20-0x27) - I/O expander 16-bit
- **PCF8574** (0x27) - I/O expander 8-bit

## 📊 Ví dụ sử dụng hoàn chỉnh

```cpp
void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    // Thiết lập callbacks cho thiết bị I2C
    i2cScanner.setDeviceCallback(DEVICE_BME280, [&]() -> bool {
        return bme280.begin();
    });
    
    i2cScanner.setDeviceCallback(DEVICE_QMC5883L, [&]() -> bool {
        return compass.begin();
    });
    
    // Thiết lập handlers
    i2cScanner.setErrorHandler([](DeviceType deviceType, const char* error) {
        Serial.printf("Lỗi: %s\n", error);
    });
    
    // Quét và kiểm tra thiết bị I2C
    i2cScanner.scanAndPrint();
    i2cScanner.checkRequiredDevices();
    i2cScanner.initializeDevices();
    
    // Khởi tạo MPU6500 qua SPI (không qua I2C Scanner)
    if (!mpu.begin()) {
        Serial.println("Lỗi: MPU6500 (SPI) không hoạt động!");
        while(1) { delay(1000); }
    } else {
        Serial.println("✓ MPU6500 (SPI): Khởi tạo thành công");
    }
    
    // Kiểm tra thiết bị I2C
    if (!i2cScanner.isDeviceWorking(DEVICE_QMC5883L)) {
        Serial.println("Cảnh báo: QMC5883L không được tìm thấy!");
    }
    
    if (!i2cScanner.isDeviceWorking(DEVICE_BME280)) {
        Serial.println("Cảnh báo: BME280 không được tìm thấy!");
    }
    
    // In trạng thái cuối cùng
    i2cScanner.printDeviceStatus();
}
```

## 🎯 Lợi ích so với phiên bản cũ

### Trước đây:
```cpp
// Code thủ công, khó bảo trì
mpu.begin(); // SPI
if (!compass.begin()) { // I2C
    Serial.println("Lỗi khởi tạo QMC5883L!");
    while (1) { /* ... */ }
}
if (!bme280.begin()) { // I2C
    Serial.println("Không tìm thấy BME280!");
}
```

### Bây giờ:
```cpp
// Code chuyên nghiệp, tự động cho I2C
i2cScanner.setDeviceCallback(DEVICE_QMC5883L, [&]() -> bool { return compass.begin(); });
i2cScanner.setDeviceCallback(DEVICE_BME280, [&]() -> bool { return bme280.begin(); });

i2cScanner.checkRequiredDevices();
i2cScanner.initializeDevices();

// MPU6500 vẫn khởi tạo thủ công qua SPI
if (!mpu.begin()) {
    Serial.println("Lỗi: MPU6500 (SPI) không hoạt động!");
}
```

## 🔄 Mở rộng

### Thêm thiết bị mới:
1. Thêm vào enum `DeviceType`
2. Cập nhật mảng `commonDevices` trong `I2CScanner.cpp`
3. Thêm vào `initializeRequiredDevicesList()` nếu cần

### Tùy chỉnh thiết bị cần thiết:
```cpp
// Trong initializeRequiredDevicesList()
requiredDevices[0] = {0x68, DEVICE_MPU6500, "MPU6500", "6-axis motion sensor", true, false, false};
//                                                                                ^^^^
//                                                                           isRequired = true
```

## 📈 Kết quả

1. **Code sạch hơn**: Không còn code kiểm tra thủ công
2. **Tự động hóa**: Khởi tạo thiết bị tự động
3. **Error handling**: Xử lý lỗi chuyên nghiệp
4. **Trạng thái rõ ràng**: Biết chính xác thiết bị nào hoạt động
5. **Dễ bảo trì**: Thêm/sửa thiết bị dễ dàng
6. **Chuyên nghiệp**: API chuẩn, tài liệu đầy đủ

Thư viện này giúp code drone trở nên chuyên nghiệp và dễ bảo trì hơn rất nhiều! 🚁 