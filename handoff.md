# Handoff

## Summary of Changes
- Refactored `main.cpp` to use OOP for LED indicators (`ESP32LEDIndicator`), removing direct `digitalWrite` and blocking delays.
- Implemented a non-blocking state machine for the status LED to reflect drone states (Heartbeat in config/disarmed, Solid in armed, Rapid blinking for low battery).
- Fixed the issue where the LED flashed rapidly in config mode while connected only via USB. `ADCBatteryMonitor::isLow()` was updated to ignore near-zero voltages (e.g., < 2.0V) which indicate the absence of a physical battery rather than a low battery condition.

## Current System State
- The flight controller is able to boot.
- The web configuration portal is accessible when in config mode.
- LED indicates Heartbeat (disarmed/config) and no longer gives false low battery warnings on pure USB power.
- Unit tests compile properly with `test_build_src = yes`.

## Verification & Testing
- Source code analysis and logic updates in `ADCBatteryMonitor.cpp`.
- Firmware successfully compiles.

## Next Steps
- User to verify the new LED logic (Heartbeat pulse) on the hardware while in Config mode.
- Continue testing motor control or IMU calibration features via the config portal.
