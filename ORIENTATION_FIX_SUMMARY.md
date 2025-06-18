# Sửa lỗi Orientation NaN - Tóm tắt thay đổi

## Vấn đề phát hiện
- Các giá trị orientation (roll, pitch, yaw) hiển thị `nan` (Not a Number)
- Nguyên nhân: Gyro bị chuyển đổi 2 lần từ độ sang radian
- Thiếu kiểm tra lỗi trong Mahony filter

## Các thay đổi đã thực hiện

### 1. Sửa lỗi chuyển đổi gyro (SimpleMahony.cpp)
```cpp
// TRƯỚC: Chuyển đổi gyro 2 lần
gx *= 0.0174533f;  // Lần 1 trong SimpleMahony
// Lần 2 trong main.cpp: gx = mpu.getGyroX() * DEG_TO_RAD;

// SAU: Xóa chuyển đổi trong SimpleMahony
// Note: gx, gy, gz are already in radians/sec from main.cpp
// No need to convert again
```

### 2. Thêm định nghĩa constants (SimpleMahony.h)
```cpp
// Conversion constants
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / PI)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI / 180.0f)
#endif
```

### 3. Thêm kiểm tra lỗi sensor data (SimpleMahony.cpp)
```cpp
// Check for invalid sensor data
if (isnan(gx) || isnan(gy) || isnan(gz) || 
    isnan(ax) || isnan(ay) || isnan(az) ||
    isnan(mx) || isnan(my) || isnan(mz)) {
    return; // Skip update if any sensor data is invalid
}
```

### 4. Thêm kiểm tra normalize quaternion (SimpleMahony.cpp)
```cpp
// Check for division by zero
if (isnan(recipNorm) || isinf(recipNorm) || recipNorm == 0.0f) {
    // Reset quaternion if normalization fails
    q0 = 1.0f; q1 = 0.0f; q2 = 0.0f; q3 = 0.0f;
    return;
}
```

### 5. Thêm kiểm tra computeAngles (SimpleMahony.cpp)
```cpp
void SimpleMahony::computeAngles() {
    // Check for valid quaternion before computing angles
    if (isnan(q0) || isnan(q1) || isnan(q2) || isnan(q3)) {
        roll = pitch = yaw = 0.0f;
        return;
    }
    
    // ... tính toán angles ...
    
    // Check for invalid angles
    if (isnan(roll) || isnan(pitch) || isnan(yaw)) {
        roll = pitch = yaw = 0.0f;
    }
}
```

### 6. Thêm debug và xử lý NaN trong main.cpp
```cpp
// Debug: Kiểm tra dữ liệu sensor mỗi 5 giây
if (millis() - lastSensorDebug >= 5000) {
    Serial.printf("Sensor Debug - Accel: ax=%.3f ay=%.3f az=%.3f\n", ax, ay, az);
    Serial.printf("Sensor Debug - Gyro: gx=%.3f gy=%.3f gz=%.3f\n", gx, gy, gz);
    
    if (isnan(ax) || isnan(ay) || isnan(az) || isnan(gx) || isnan(gy) || isnan(gz)) {
        Serial.println("LỖI: Dữ liệu sensor có giá trị NaN!");
    }
}

// Kiểm tra và xử lý dữ liệu NaN
if (isnan(ax) || isnan(ay) || isnan(az)) {
    ax = 0.0f; ay = 0.0f; az = 1.0f; // Giá trị mặc định
}
if (isnan(gx) || isnan(gy) || isnan(gz)) {
    gx = 0.0f; gy = 0.0f; gz = 0.0f; // Giá trị mặc định
}
```

### 7. Điều chỉnh gains Mahony filter
```cpp
// Giảm Kp từ 5.0 xuống 2.0 để ổn định hơn
mahony.setGains(2.0f, 0.0f, 1.0f);
```

## Kết quả mong đợi
- Loại bỏ hoàn toàn giá trị NaN trong orientation
- Cải thiện độ ổn định của Mahony filter
- Thêm debug để dễ dàng phát hiện vấn đề
- Xử lý graceful khi có lỗi sensor

## Cách test
1. Upload code lên ESP32
2. Mở Serial Monitor (115200 baud)
3. Quan sát debug output mỗi 5 giây
4. Kiểm tra WebSocket debug data không còn NaN

## Lưu ý
- Nếu vẫn có NaN, kiểm tra kết nối sensor
- Có thể cần điều chỉnh gains tùy theo ứng dụng
- Debug output sẽ giúp xác định nguyên nhân cụ thể 