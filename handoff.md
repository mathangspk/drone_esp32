# Project Handoff - ESP32 Flight Controller C++ OOP Rewrite

This document records the current progress, state, and next steps of the project.

---

## Summary of Changes
- **Project Structure**: Setup PlatformIO project with double environment support: `esp32dev` (target hardware) and `native` (host testing).
- **C++ Coding Standard (`agent.md`)**: Configured strict modern C++17 rules, standard OOP naming conventions, a **strict limit of 100 lines per file**, and doctest TDD workflow. Fixed Vietnamese comments to satisfy the English-only guideline.
- **System Architecture (`architecture.md`)**: Visualized class relationships, abstract interfaces, and targets.
- **Hardware Abstraction Layer (HAL)**: Created clean, platform-independent virtual interfaces (`IIMU`, `IPPM`, `IMotors`, `IBattery`).
- **Core Algorithms**:
  - `PIDController`: Dynamic gain tuning support, proportional, trapezoidal integral with anti-windup clamping, and derivative outputs.
  - `KalmanFilter`: 1D Kalman equations to fuse gyroscope rates and accelerometer angles.
  - `FlightController`: Supports safety arm switch (AUX1), dynamic PID loading, and RAM Blackbox logging.
- **Physical Drivers (ESP32)**:
  - `MPU6500IMU`: SPI IMU reader (SCK=18, MISO=19, MOSI=23, CS=5). Runs at high-speed SPI.
  - `IBusReceiverDriver`: Digital serial i-BUS receiver (UART2 on pin 16). Handles channel parsing and timeout failsafe.
  - `QMC5883LCompass`: Auxiliary I2C compass sensor (address 0x0D) to measure heading.
  - `PWMESP32Motors`: LEDC hardware PWM speed writer (250Hz frequency) mapping to **M1=25, M2=27, M3=4, M4=14** to avoid conflicts.
  - `ADCBatteryMonitor`: Voltage divider analog pin reader mapping to **pin 33**.
- **Web Config Dashboard & Telemetry Logs**:
  - `WebDashboardPage`: Static HTML dashboard page with sliders and fields.
  - `WebDashboardServer`: SoftAP (`ESP32_Drone_Config`) server that activates *only* in the DISARMED state, and powers down the Wi-Fi stack completely during flight (ARMED state) to avoid CPU interrupts.
  - **Web Dashboard Controllers**: Implements PID tuning via ESP32 `Preferences` (NVS), safe motor testing (capped at 1150us), and formats CSV logs. Added **Joystick Overrides** to allow virtual joystick simulations over Wi-Fi when DISARMED.
  - **RAM Blackbox Logger**: Continuous 50Hz circular buffer logging of Roll/Pitch setpoints and actual values, throttle, and voltages, frozen on disarm for easy copy-pasting.
- **Redraw Hardware Connection Diagram**: Updated [architecture.md](file:///c:/local/opencode/iot/esp32_drone/architecture.md) with a clear ASCII-art and tabular layout mapping ESP32 pinouts for MPU6500 (SPI), QMC5883L (I2C), Flysky i-BUS receiver (UART2), Voltage Monitor divider (GPIO 33), and LEDC ESC outputs (GPIO 25, 27, 4, 14).
- **Algorithm & Driver Safety Fixes**:
  - **MPU6500 Init**: Added full device reset (`0x80→0x6B`), 100ms startup delays, and DLPF_CFG=3 (Gyro BW 41Hz) to filter motor vibration noise.
  - **Angle PID D-term**: Changed default `DAngleRoll` and `DAnglePitch` from `0.6` to `0.0` to prevent derivative noise amplification on first flights. Tunable via Web Dashboard.
  - **Gyro Calibration**: Added `delayMicroseconds(1000)` between each sample to ensure 2000 genuinely independent IMU measurements instead of reading duplicate data.
  - **Battery Voltage Filter**: Added an Exponential Moving Average (EMA) filter (alpha = 0.05) to eliminate high-frequency ADC noise, with a 20-sample warmup sequence during `init()` to establish a correct baseline immediately on boot.
  - **Safe Battery Telemetry**: Removed the instantaneous low battery shutdown (`voltage < 9.0f`) from `FlightController::update` to prevent dangerous mid-air motor cutoff caused by transient voltage sag under heavy load (e.g. A2212 1400KV drawing ~50-60A total on 3S LiPo).

---

## Current System State
- **Compiling State**: The entire codebase compiles successfully for the `esp32dev` target! Flash usage is 63.6%, RAM usage is 17.8%.
- **Testing**: All unit tests build and pass cleanly on the native compiler environment.
- **Optimal Hardware Configuration**:
  - Drone Takeoff Weight: 800g (highly optimized).
  - Recommended Propellers: 8045 (8-inch) props for optimal motor load and cool operation.
  - Recommended Battery: 3S 2200mAh - 3300mAh LiPo battery (180g - 260g) for 8 to 11 minutes of flight.
  - Telemetry Alarms: Telemetry/LED warning triggers at < 9.0V (3.0V/cell critical threshold) using a filtered and sag-resistant value.

---

## Verification & Testing
- **Target Verification**: Compiled the entire codebase via PlatformIO for target ESP32 Dev Module.
  - **Outcome**: `[SUCCESS] Took 9.20 seconds`. Zero compilation warnings or errors.
- **Native Unit Tests**: `pio test -e native` — `[PASSED] Took 4.05 seconds`.

---

## Next Steps
1. **Flash Firmware**: Connect ESP32 to the PC and flash the compiled `firmware.bin` via USB.
2. **First Power-Up & Web Config**:
   - Turn on the transmitter (Flysky) and make sure AUX1 is in the low position (DISARMED).
   - Power up the drone. Connect your PC/phone to the Wi-Fi access point `ESP32_Drone_Config` (password: `12345678`).
   - Open a browser and go to `http://192.168.4.1/`.
   - Verify transmitter stick movements in the Receiver tab.
   - Run a Motor Test (at 1050-1100us) to verify correct rotation direction of each motor.
3. **Flight Test & PID Copy-Paste**:
   - Arm the drone via AUX1 (Wi-Fi will shut down automatically).
   - Perform a short hover test (10-20 seconds).
   - Land and Disarm the drone.
   - Reconnect to the Wi-Fi AP, open the dashboard, click **Fetch CSV Log**, copy the text, and paste it to the AI chat to get optimized PID recommendations!
4. **Tuning and Hardware Additions**:
   - Consider connecting an active buzzer (5V) to the LED alarm pin (GPIO 2) or a dedicated pin to alert you audibly when battery drops below 9.0V, as the built-in LED is invisible during high-altitude flights.
