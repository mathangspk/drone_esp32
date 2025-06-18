# So sánh với Betaflight - Hướng dẫn cải thiện

## Vấn đề hiện tại
- Roll/pitch thay đổi ít khi di chuyển drone
- Thiếu calibration và proper filtering
- Sensor fusion chưa tối ưu

## Betaflight vs Code hiện tại

### 1. Sensor Fusion Algorithm
| Betaflight | Code hiện tại |
|------------|---------------|
| Madgwick filter | Mahony filter |
| Beta parameter tuning | Fixed gains |
| 8kHz sample rate | 100Hz sample rate |

### 2. Gyro Processing
| Betaflight | Code hiện tại |
|------------|---------------|
| Gyro calibration | Raw gyro data |
| Bias compensation | No bias comp |
| Scale factor correction | Basic deg->rad |
| Low-pass filter | No filtering |

### 3. Accelerometer Processing
| Betaflight | Code hiện tại |
|------------|---------------|
| Calibration (offset/scale) | Raw accel data |
| Gravity compensation | No gravity comp |
| Low-pass filter | No filtering |

### 4. Magnetometer Processing
| Betaflight | Code hiện tại |
|------------|---------------|
| Hard/soft iron calibration | Raw mag data |
| Declination correction | No declination |
| Low-pass filter | No filtering |

## Cải thiện theo Betaflight

### 1. Thêm Gyro Calibration
```cpp
// Gyro bias calibration
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;
void calibrateGyro() {
    // Collect samples when drone is stationary
    // Calculate average bias
    // Apply bias compensation
}
```

### 2. Thêm Accelerometer Calibration
```cpp
// Accel calibration (6-point calibration)
void calibrateAccel() {
    // Face up, face down, left, right, front, back
    // Calculate offset and scale factors
}
```

### 3. Thêm Magnetometer Calibration
```cpp
// Mag calibration (12-point calibration)
void calibrateMag() {
    // Rotate drone in all directions
    // Calculate hard/soft iron compensation
}
```

### 4. Cải thiện Filter Tuning
```cpp
// Tăng sample rate và tune parameters
#define SAMPLE_FREQ 1000.0f  // 1kHz thay vì 100Hz
mahony.setGains(5.0f, 0.1f, 1.0f);  // Tăng Kp và thêm Ki
```

### 5. Thêm Low-pass Filter
```cpp
// Simple low-pass filter
float lowPassFilter(float input, float output, float alpha) {
    return alpha * input + (1.0f - alpha) * output;
}
```

## Kế hoạch triển khai

### Phase 1: Basic Calibration
1. Thêm gyro bias calibration
2. Thêm accel 6-point calibration
3. Thêm mag 12-point calibration

### Phase 2: Filtering
1. Thêm low-pass filter cho gyro
2. Thêm low-pass filter cho accel
3. Thêm low-pass filter cho mag

### Phase 3: Advanced Features
1. Tăng sample rate
2. Tune filter parameters
3. Thêm temperature compensation

## Commands để test
```
CALIBRATE_GYRO    - Calibrate gyro bias
CALIBRATE_ACCEL   - Calibrate accelerometer
CALIBRATE_MAG     - Calibrate magnetometer
SET_FILTER_GAINS  - Set filter parameters
RESET_ALL         - Reset all calibrations
```

## Expected Results
Sau khi cải thiện:
- Roll/pitch sẽ thay đổi đúng với góc nghiêng thực tế
- Yaw sẽ ổn định và chính xác
- Sensor fusion sẽ mượt mà hơn
- Ít noise và drift 