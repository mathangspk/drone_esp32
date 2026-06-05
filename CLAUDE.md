# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build for ESP32 hardware target
pio run -e esp32dev

# Build and flash to connected ESP32
pio run -e esp32dev --target upload

# Run all unit tests on the host (native)
pio test -e native

# Run a single test file
pio test -e native --filter "tdd/test_pid"

# Open serial monitor (115200 baud)
pio device monitor
```

## Architecture

This is an ESP32 drone flight controller written in C++17 with PlatformIO. Two build environments exist:
- `esp32dev` — real hardware target (ESP32-WROOM)
- `native` — host OS build used exclusively for unit testing with doctest

### Layered Design

```
main.cpp
  └── FlightController (core algorithm)
        ├── IIMU / IPPM / IMotors / IBattery  ← abstract interfaces
        │     └── MPU6500IMU / IBusReceiverDriver / PWMESP32Motors / ADCBatteryMonitor  ← ESP32 drivers
        ├── PIDController (cascaded angle → rate)
        └── KalmanFilter (gyro + accel fusion)
```

All hardware drivers implement platform-independent abstract interfaces (`include/interfaces/`). The `SimulatedHardware.h` provides mock implementations for native testing. Platform-specific code (`#ifndef NATIVE_BUILD`) is strictly isolated to driver files and `main.cpp`.

### FreeRTOS Tasks (main.cpp)

Three tasks pinned to specific cores:
- **Battery Task** (Core 0, priority 1) — blinks GPIO 2 LED on low voltage
- **Web Task** (Core 0, priority 1) — SoftAP Wi-Fi dashboard active only when DISARMED (AUX1 channel 4 ≤ 1500)
- **Flight Task** (Core 1, priority 2) — calls `fc.update(0.004f)` at 250Hz (4ms loop)

### Flight Control Loop

`FlightController::update()` in `src/core/FlightController.cpp` runs a cascaded PID cascade:
1. Read IMU + PPM channels
2. Apply Kalman filter to fuse gyro rates and accelerometer angles
3. Outer angle PID: desired angle → desired rate
4. Inner rate PID: desired rate → motor correction
5. Motor mixing formula: `1.024 * (throttle ± roll ± pitch ± yaw)` per motor

PID gains load from ESP32 NVS (`Preferences`) at arm-time via `loadPIDGains()`, with fallback to hardcoded defaults.

### Web Dashboard

`WebDashboardServer` creates a SoftAP (`ESP32_Drone_Config`, password `12345678`) at `http://192.168.4.1/`. Wi-Fi is fully shut down when the drone arms. The dashboard supports live receiver monitoring, safe motor testing (capped at 1150µs), PID tuning (persisted to NVS), and CSV log export from the RAM blackbox (50Hz circular buffer, frozen on disarm).

## Coding Standards (from agent.md & ESP32_CODING_STANDARDS_EN.md)

**Strict 100-line limit per file** — if a `.h` or `.cpp` file approaches 100 lines, split it into smaller single-responsibility units (see `FlightController.cpp` + `FlightControllerPID.cpp` as an example).

- C++17 throughout
- `PascalCase` for classes, `camelCase` for methods/variables, `UPPER_CASE` for constants
- Interface headers prefixed with `I` (e.g., `IIMU`, `IPPM`)
- No global variables; encapsulate all state in classes
- No `throw` on embedded targets — use boolean returns or `std::optional`
- Prefer stack allocation and pass-by-const-reference; use smart pointers if heap is needed
- Comments explain **why**, not what

**ESP32 Firmware Mandates:**
You MUST adhere to the standards outlined in `ESP32_CODING_STANDARDS_EN.md`:
- **Zero Blocking**: Never use `delay()`. Use `vTaskDelay()` or state machines.
- **No Dynamic Strings**: Never use the Arduino `String` class. Use `char[]`.
- **Strict ISR Rules**: ISRs must be extremely short, use `IRAM_ATTR`, and only call `FromISR` APIs.
- **Error Handling**: You must check `esp_err_t` return values.

## TDD Workflow

New features require tests written first. Tests live in `tests/tdd/` and use **doctest**. The `tests/test_main.cpp` provides the doctest entry point.

1. Write test in `tests/tdd/test_<feature>.cpp`
2. Get user approval on test cases
3. Implement the minimal code to pass
4. Verify: `pio test -e native`

## Hardware Pin Reference

| Device | Interface | Key Pins |
|---|---|---|
| MPU6500 IMU | SPI (VSPI) | SCK=18, MISO=19, MOSI=23, CS=5 |
| QMC5883L Compass | I2C | SCL=22, SDA=21 |
| Flysky i-BUS RX | UART2 | RX=GPIO16 |
| Battery Monitor | ADC | GPIO33 (voltage divider: 77.6kΩ / 29.4kΩ) |
| Low Battery LED | GPIO | GPIO2 |
| Motors (ESC PWM) | LEDC | M1=25, M2=27, M3=4, M4=14 |

## Maintenance

After completing any functional milestone, update:
- `architecture.md` — class tree and Mermaid diagrams
- `handoff.md` — current state and next steps
