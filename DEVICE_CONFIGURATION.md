# Cấu hình Thiết bị Drone - Chính xác

## 🔌 Kết nối thiết bị

### 📡 Thiết bị I2C (được quản lý bởi I2CScanner):

| Thiết bị | Địa chỉ I2C | Chức năng | Trạng thái |
|----------|-------------|-----------|------------|
| **BME280** | 0x76 | Cảm biến nhiệt độ, áp suất, độ ẩm | Tùy chọn |
| **QMC5883L** | 0x0D | Cảm biến từ trường 3 trục | Tùy chọn |
| **HMC5883L** | 0x1E | Cảm biến từ trường 3 trục (backup) | Tùy chọn |

### 🔌 Thiết bị SPI (không được quản lý bởi I2CScanner):

| Thiết bị | Chân SPI | Chức năng | Trạng thái |
|----------|----------|-----------|------------|
| **MPU6500** | CS: 5 | Cảm biến chuyển động 6 trục | **Bắt buộc** |

### 🔌 Thiết bị khác:

| Thiết bị | Kết nối | Chức năng | Trạng thái |
|----------|---------|-----------|------------|
| **IBus Receiver** | UART2 (RX: 16) | Nhận tín hiệu điều khiển | Bắt buộc |
| **ESC Controllers** | PWM (25, 27, 4, 14) | Điều khiển động cơ | Bắt buộc |
| **Battery Monitor** | ADC (33) | Đo điện áp pin | Bắt buộc |
| **Current Monitor** | ADC (34) | Đo dòng điện | Bắt buộc |

## 🎯 Tại sao MPU6500 không có trong I2CScanner?

### Lý do kỹ thuật:
1. **Kết nối SPI**: MPU6500 sử dụng giao thức SPI, không phải I2C
2. **Thư viện I2CScanner**: Chỉ quản lý thiết bị I2C
3. **Khởi tạo riêng biệt**: MPU6500 cần được khởi tạo thủ công qua SPI

### Code thực tế:
```cpp
// MPU6500 qua SPI - khởi tạo và kiểm tra
mpu.begin(); // Khởi tạo SPI
if (!mpu.isConnected()) { // Kiểm tra kết nối
    Serial.println("Lỗi: MPU6500 (SPI) không hoạt động!");
    while (1) {
        statusLed.blink(100, 3);
        vTaskDelay(1000);
    }
} else {
    Serial.println("✓ MPU6500 (SPI): Khởi tạo thành công");
}

// Thiết bị I2C - quản lý tự động
i2cScanner.setDeviceCallback(DEVICE_BME280, [&]() -> bool { return bme280.begin(); });
i2cScanner.setDeviceCallback(DEVICE_QMC5883L, [&]() -> bool { return compass.begin(); });
i2cScanner.initializeDevices();
```

## 📊 Phân loại thiết bị

### 🔴 Thiết bị bắt buộc (Required):
- **MPU6500** (SPI) - Cảm biến chuyển động 6 trục
- **IBus Receiver** (UART) - Nhận tín hiệu điều khiển
- **ESC Controllers** (PWM) - Điều khiển động cơ
- **Battery Monitor** (ADC) - Đo điện áp pin
- **Current Monitor** (ADC) - Đo dòng điện

### 🟡 Thiết bị tùy chọn (Optional):
- **BME280** (I2C) - Cảm biến nhiệt độ, áp suất, độ ẩm
- **QMC5883L** (I2C) - Cảm biến từ trường 3 trục
- **HMC5883L** (I2C) - Cảm biến từ trường 3 trục (backup)

## 🔧 Cách hoạt động

### 1. **Khởi tạo I2C Scanner**:
```cpp
// Quét và kiểm tra thiết bị I2C
i2cScanner.scanAndPrint();
i2cScanner.checkRequiredDevices();
i2cScanner.initializeDevices();
```

### 2. **Khởi tạo MPU6500 SPI**:
```cpp
// Khởi tạo thủ công qua SPI
if (!mpu.begin()) {
    // Xử lý lỗi
}
```

### 3. **Kiểm tra kết quả**:
```cpp
// In trạng thái thiết bị I2C
i2cScanner.printDeviceStatus();

// Kiểm tra thiết bị I2C cụ thể
if (i2cScanner.isDeviceWorking(DEVICE_QMC5883L)) {
    // QMC5883L hoạt động
}
```

## 🎯 Lợi ích của cấu hình này

### 1. **Phân chia trách nhiệm rõ ràng**:
- I2CScanner quản lý thiết bị I2C
- SPI thiết bị khởi tạo riêng biệt

### 2. **Linh hoạt**:
- Có thể thêm thiết bị I2C mới dễ dàng
- MPU6500 vẫn hoạt động bình thường qua SPI

### 3. **Debugging dễ dàng**:
- Biết chính xác thiết bị nào qua I2C
- Biết thiết bị nào qua SPI
- Thông báo lỗi rõ ràng

### 4. **Bảo trì đơn giản**:
- Thêm thiết bị I2C mới chỉ cần cập nhật I2CScanner
- Thiết bị SPI không ảnh hưởng đến I2CScanner

## 📝 Ghi chú quan trọng

1. **MPU6500 qua SPI**: Không thể quản lý bằng I2CScanner
2. **BME280 và QMC5883L**: Được quản lý hoàn toàn bởi I2CScanner
3. **HMC5883L**: Thiết bị backup, có thể thay thế QMC5883L
4. **Tất cả thiết bị I2C**: Đều là tùy chọn, không bắt buộc cho hoạt động cơ bản

Cấu hình này đảm bảo drone hoạt động ổn định với các thiết bị cần thiết, đồng thời cho phép mở rộng dễ dàng với các thiết bị I2C bổ sung! 🚁 