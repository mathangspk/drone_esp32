# Drone Debug Guide

## Tổng quan
Dự án drone ESP32 này có nhiều cách để debug và phân tích dữ liệu real-time.

## Chu kỳ thu thập dữ liệu

### 1. Vòng lặp điều khiển chính
- **Tần số:** 500Hz (2ms)
- **Chức năng:** Đọc cảm biến, cập nhật PID, điều khiển motor

### 2. Serial Debug
- **Config mode:** 10Hz (100ms) - Dữ liệu orientation/receiver
- **Operator mode:** 50Hz (20ms) - Dữ liệu điều khiển đầy đủ

### 3. Web Debug
- **HTTP API:** Theo yêu cầu
- **WebSocket:** 20Hz (50ms) - Real-time plotting
- **Web Interface:** 10Hz (100ms) - Chart.js plotting

## Cách sử dụng Debug

### 1. Serial Debug (Python Script)

#### Cài đặt:
```bash
pip install pyserial
```

#### Sử dụng:
```bash
python serial_debug.py
```

#### Chế độ:
- **Config (1):** Hiển thị dữ liệu cảm biến
  - Orientation: Roll, Pitch, Yaw
  - Receiver: Throttle, Roll, Pitch, Yaw, AUX1, AUX2
- **Operator (2):** Hiển thị dữ liệu điều khiển đầy đủ

### 2. Web Interface

#### Kết nối:
1. Kết nối WiFi ESP32 (SSID: "ESP32-WIFI", Password: "12345678")
2. Mở trình duyệt: `http://192.168.4.1`

#### Các trang:
- **Trang chủ:** Điều khiển ESC và xem dữ liệu cơ bản
- **Plotter:** `http://192.168.4.1/plotter` - Biểu đồ real-time

### 3. Python Plotter

#### Cài đặt:
```bash
pip install -r requirements.txt
```

#### Sử dụng:
```bash
python debug_plotter.py
```

#### Tính năng:
- 4 biểu đồ real-time
- Dữ liệu: Attitude, PID, Motor, System
- Tần số: 10Hz

### 4. API Endpoints

#### Các endpoint có sẵn:
- `/debug` - Tất cả dữ liệu debug
- `/attitude` - Góc roll, pitch, yaw
- `/pid` - Output PID controllers
- `/motors` - Giá trị motor
- `/receiver` - Dữ liệu receiver
- `/sensor` - Dữ liệu MPU6500
- `/battery` - Điện áp pin
- `/current` - Dòng điện
- `/bme` - Áp suất BME280

#### Ví dụ sử dụng:
```bash
curl http://192.168.4.1/debug
```

## Dữ liệu có thể phân tích

### 1. Attitude Data
- **Roll:** Góc nghiêng trái/phải
- **Pitch:** Góc nghiêng trước/sau
- **Yaw:** Góc xoay

### 2. PID Data
- **Roll PID:** Output điều khiển roll
- **Pitch PID:** Output điều khiển pitch
- **Yaw PID:** Output điều khiển yaw

### 3. Motor Data
- **Motor 1-4:** Giá trị PWM (0-100%)

### 4. System Data
- **Voltage:** Điện áp pin
- **Current:** Dòng điện tiêu thụ

### 5. Sensor Data
- **Accelerometer:** Gia tốc X, Y, Z
- **Gyroscope:** Tốc độ góc X, Y, Z
- **Temperature:** Nhiệt độ

## Tune PID

### 1. Các tham số hiện tại:
```cpp
#define ROLL_KP 4.0f
#define ROLL_KI 0.1f
#define ROLL_KD 0.2f

#define PITCH_KP 4.0f
#define PITCH_KI 0.1f
#define PITCH_KD 0.2f

#define YAW_KP 3.0f
#define YAW_KI 0.1f
#define YAW_KD 0.1f
```

### 2. Quy trình tune:
1. **Tune P trước:** Tăng Kp cho đến khi có dao động nhẹ
2. **Tune D:** Tăng Kd để giảm dao động
3. **Tune I:** Tăng Ki để loại bỏ steady-state error

### 3. Dấu hiệu cần tune:
- **Oscillation:** Giảm Kp, tăng Kd
- **Slow response:** Tăng Kp
- **Steady-state error:** Tăng Ki
- **Overshoot:** Giảm Kp, tăng Kd

## Troubleshooting

### 1. Không kết nối được WiFi
- Kiểm tra SSID và password
- Đảm bảo ESP32 đã khởi động hoàn toàn

### 2. Không nhận được dữ liệu
- Kiểm tra kết nối cảm biến
- Xem Serial Monitor để debug

### 3. Biểu đồ không cập nhật
- Kiểm tra kết nối mạng
- Đảm bảo ESP32 đang chạy

### 4. PID không ổn định
- Giảm các giá trị Kp, Ki, Kd
- Kiểm tra cân bằng drone
- Kiểm tra motor và propeller

## Lưu ý quan trọng

1. **Test trên mặt đất trước khi bay**
2. **Luôn có cơ chế dừng khẩn cấp**
3. **Kiểm tra pin trước mỗi lần bay**
4. **Tune PID từng bước một**
5. **Lưu backup các tham số đã tune tốt** 