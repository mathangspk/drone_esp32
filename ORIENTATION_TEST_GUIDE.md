# Hướng dẫn Test và Debug Orientation

## Vấn đề hiện tại
- Roll và yaw gần 0° (bình thường khi drone nằm phẳng)
- Pitch có giá trị -0.4° (có thể do mounting hoặc calibration)

## Các bước test

### 1. Upload code và mở Serial Monitor
```bash
# Upload code lên ESP32
platformio run --target upload

# Mở Serial Monitor (115200 baud)
platformio device monitor
```

### 2. Quan sát Calibration Test
Khi khởi động, hệ thống sẽ tự động chạy calibration test:
```
=== Calibration Test ===
Đặt drone nằm phẳng và đứng yên trong 3 giây...
Initial Accel: ax=0.123 ay=-0.456 az=9.789
Initial Gyro: gx=0.001 gy=0.002 gz=0.003
Mahony filter đã reset - bắt đầu tracking...
```

### 3. Kiểm tra Debug Output (mỗi 5 giây)
```
Sensor Debug - Accel: ax=0.123 ay=-0.456 az=9.789
Sensor Debug - Gyro: gx=0.001 gy=0.002 gz=0.003
Sensor Debug - Mag: mx=0.123 my=0.456 mz=-0.789
Accel Magnitude: 9.812 (should be ~9.81)
Gyro Bias Check - gx=0.001 gy=0.002 gz=0.003

Orientation Debug - Roll=-0.040 Pitch=-0.430 Yaw=-0.020
✓ Drone đang nằm phẳng (roll và pitch < 1°)
✓ Yaw ổn định
```

### 4. Commands để test

#### Reset Mahony Filter
```
RESET_MAHONY
```
- Reset quaternion về trạng thái ban đầu
- Dùng khi orientation bị lỗi

#### Calibration Manual
```
CALIBRATE
```
- Chạy calibration thủ công
- Đặt drone nằm phẳng và đứng yên

### 5. Kiểm tra các giá trị

#### Accelerometer (nên gần 9.81 m/s²)
- **ax, ay**: Gần 0 khi nằm phẳng
- **az**: Gần 9.81 (1g)
- **Magnitude**: √(ax² + ay² + az²) ≈ 9.81

#### Gyroscope (nên gần 0 khi đứng yên)
- **gx, gy, gz**: Gần 0 rad/s khi đứng yên
- Nếu có bias lớn → cần calibration

#### Magnetometer
- **mx, my, mz**: Giá trị từ trường
- Cần để xác định yaw

### 6. Test các tư thế khác nhau

#### Test 1: Nằm phẳng
- Roll ≈ 0°, Pitch ≈ 0°
- Đây là trạng thái bình thường

#### Test 2: Nghiêng trái/phải
- Roll sẽ thay đổi ±10-30°
- Pitch vẫn gần 0°

#### Test 3: Nghiêng trước/sau
- Pitch sẽ thay đổi ±10-30°
- Roll vẫn gần 0°

#### Test 4: Xoay yaw
- Yaw sẽ thay đổi 0-360°
- Roll và pitch không đổi

### 7. Troubleshooting

#### Nếu roll/pitch luôn = 0
1. Kiểm tra kết nối MPU6500
2. Kiểm tra mounting sensor
3. Chạy `CALIBRATE` command

#### Nếu yaw không ổn định
1. Kiểm tra kết nối magnetometer
2. Tránh nhiễu từ trường
3. Chạy `RESET_MAHONY`

#### Nếu có NaN
1. Kiểm tra dữ liệu sensor
2. Chạy `RESET_MAHONY`
3. Kiểm tra kết nối hardware

### 8. Giá trị mong đợi

#### Khi drone nằm phẳng:
```
Roll: -1° đến +1°
Pitch: -1° đến +1°
Yaw: Ổn định (thay đổi < 0.1°/s)
```

#### Khi nghiêng 15°:
```
Roll hoặc Pitch: ±15° ±1°
Yaw: Ổn định
```

### 9. Lưu ý quan trọng
- **Drone phải đứng yên** khi test
- **Tránh nhiễu từ trường** (motor, dây điện)
- **Mounting sensor phải chính xác**
- **Calibration cần thực hiện ở nhiệt độ ổn định** 