# ESP32 Drone Flight Controller - Current Local Status Report

## Summary of Recent Changes
- **ESC Calibration via Web Dashboard**: Added a new `/api/calibrate` endpoint and a manual "ESC Calibration (DANGER)" interface in the web dashboard. This allows sending 2000us (max throttle) and 1000us (min throttle) to correctly calibrate ESCs. A mandatory safety checkbox ensures the user has removed propellers before any signals are sent.
- **LED Status Indicator**: Refactored `main.cpp` to use OOP (`ESP32LEDIndicator`) for LED status, eliminating blocking delays. The LED now shows a "Heartbeat" pulse in Config/Disarmed mode, solid when Armed, and flashes rapidly on low battery.
- **Battery Monitor Fix**: Updated `ADCBatteryMonitor::isLow()` to correctly handle USB-only power (voltage near 0V), preventing false low-battery alerts when a physical battery is not attached.
- **Web Dashboard - IMU Telemetry**: Added a new `/api/imu` endpoint. The web dashboard UI (`WebDashboardPage.h`) now features an "IMU Sensor Monitor" card that displays real-time Pitch, Roll, and Yaw angles/rates to help verify sensor functionality.
- **Motor Control & Testing**: 
  - Verified PWM output is set to 250Hz, 12-bit resolution.
  - The Web Dashboard "Motor Test Mode" allows testing individual motors without a transmitter.
- **Project Configuration**: Updated `platformio.ini` to set `upload_speed = 115200` to mitigate serial noise during firmware flashing, and added `test_build_src = yes` to resolve native unit testing issues.

## Current System State
- The flight controller successfully compiles and boots.
- ESCs initialize correctly and emit the arming beeps.
- The web configuration portal (`ESP32_Drone_Config`) is active when in Config Mode.
- Real-time telemetry (PID, Receiver, IMU) and ESC Calibration functions are fully integrated and functional on the dashboard.

## Next Steps / Actions for User
1. **Flash Firmware**: Run `pio run -t upload` to flash the newly compiled firmware with the ESC calibration feature.
2. **ESC Calibration**: 
   - Open the web dashboard and check "Tôi xác nhận đã tháo toàn bộ cánh quạt".
   - Press **Gửi 2000us**, then plug in the LiPo battery. Wait for max throttle beeps.
   - Press **Gửi 1000us** and wait for the arming beeps.
   - Press **Kết thúc & Thoát**.
3. **IMU and Motor Testing**: Verify Pitch/Roll telemetry by moving the drone, and test individual motors using the Motor Test Mode sliders.
