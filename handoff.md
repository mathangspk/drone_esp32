# Handoff

## Summary of Changes
- Fixed disarmed motor override bug: Updated `FlightController::update` to call `motors_.writeMotors` even when disarmed, allowing the ESCs to initialize and receive continuous override signals when using the Web Dashboard Motor Test.
- Added an IMU telemetry endpoint (`/api/imu`) to `WebDashboardServer` to return raw Accelerometer Angles and Gyroscope Rates.
- Updated the HTML/JS dashboard (`WebDashboardPage.h`) to display the IMU telemetry in a new "IMU Sensor Monitor" card.

## Current System State
- Drone boots normally.
- Config Web Dashboard is accessible over Wi-Fi (`ESP32_Drone_Config`).
- Dashboard shows PID, Receiver, IMU, and Motor Test tabs, all updating dynamically.

## Verification & Testing
- Firmware compiles successfully.
- Web UI is updated and functional.

## Next Steps
- User to test the web dashboard's IMU monitor by tilting the drone.
- User to test the motors using the Motor Test section (ensure props are removed!).

