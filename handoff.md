# Project Handoff — ESP32 Flight Controller

---

## Current State (2026-06-04)

Codebase is clean and passing all 59 unit test assertions. The last two sessions completed a full audit and fixed every identified issue through P3. Ready for hardware flash and test flight.

---

## What Was Built

### Hardware Abstraction Layer
Clean virtual interfaces in `include/interfaces/`: `IIMU`, `IPPM`, `IMotors`, `IBattery`. All ESP32 drivers implement these so the flight algorithm is platform-independent and fully testable on host.

### Core Algorithms (`src/core/`)
- **PIDController** — trapezoidal integral with anti-windup (±400µs), D-on-measurement with optional LPF (`dAlpha`). Output limit named `kOutputLimit`.
- **KalmanFilter** — 1D gyro + accelerometer fusion. Process noise σ=4 deg/s, measurement noise σ=3 deg.
- **FlightController** — cascaded angle→rate PID, Kalman-fused attitude, motor mixing with collective saturation clamping. Splits into `FlightController.cpp` + `FlightControllerPID.cpp` to stay under 100-line limit.

### ESP32 Drivers (`src/hardware/`)
- **MPU6500IMU** — SPI at 8MHz, full device reset on `begin()`, DLPF_CFG=3 (41Hz gyro BW to cut motor vibration).
- **IBusReceiverDriver** — i-BUS serial at 115200 baud on UART2/GPIO16. 32-byte frame, checksum verified, 100ms signal-loss timeout.
- **PWMESP32Motors** — LEDC 250Hz/12-bit. µs→duty via `usToDuty()`. Using arduino-esp32 **2.0.17** (old API: `ledcSetup`/`ledcAttachPin`/`ledcWrite(channel, duty)`).
- **ADCBatteryMonitor** — EMA filter (α=0.05), 20-sample warmup on init, `std::atomic<float>` for cross-core read safety. Single writer: Battery Task on Core 0.
- **QMC5883LCompass** — I2C auxiliary sensor. Present and initialized but **not used in the flight loop**.

### Web Dashboard (`src/network/`)
SoftAP `ESP32_Drone_Config` / `12345678` → `http://192.168.4.1/`. Active only when **DISARMED**; Wi-Fi is fully shut down on arm.

| Endpoint | Function |
|---|---|
| `GET /` | Embedded HTML dashboard |
| `GET /api/pid` | Read current PID gains (NVS or defaults) |
| `POST /api/pid` | Write PID gains to NVS (validated 0–20) |
| `GET /api/receiver` | Live RC channel values |
| `POST /api/receiver` | Virtual joystick override (disarmed only) |
| `POST /api/motor` | Safe motor test (capped 1000–1150µs) |
| `GET /api/log` | CSV export of RAM blackbox (500 entries @ 50Hz = 10s) |

### FreeRTOS Tasks
| Task | Core | Priority | Role |
|---|---|---|---|
| Battery Task | 0 | 1 | `ADCBatteryMonitor::update()` + GPIO 2 LED blink |
| Web Task | 0 | 1 | `WebDashboardServer::handleClient()` when disarmed |
| Flight Task | 1 | 2 | `FlightController::update(0.004f)` at 250Hz |

---

## Audit Fixes Applied (Sessions 2–3)

| Priority | Issue | Fix |
|---|---|---|
| P0 | `lib/drone_core/` duplicated `src/core/` causing potential double-symbol compile | Deleted `lib/drone_core/`, sole source is `src/core/` |
| P0 | Data race: `readVoltage()` had side-effects via `mutable float` on two cores | `std::atomic<float>`, separated `update()` (writer, Core 0) from `readVoltage()` (reader, any core) |
| P1 | LEDC duty cycle used raw µs as duty count (factor ~1024 wrong) | Added `usToDuty()` helper: `(µs × 4096) / 4000` |
| P1 | Loop timer reset to `micros()` each iteration causing accumulated jitter | Changed to `loopTimer += kPeriodUs` (fixed-interval advance) |
| P1 | Signal-loss timeout magic numbers `> 1000` and `> 10` | Named constants `SIGNAL_LOSS_TIMEOUT_MS=100`, `FRAME_GAP_TIMEOUT_MS=10` |
| P1 | `ARM_CHANNEL`/`ARM_THRESHOLD` private in FlightController but re-hardcoded in 3 dashboard places | Promoted to `public` in `FlightController.h`; dashboard uses `FlightController::ARM_CHANNEL/ARM_THRESHOLD` |
| P2 | Arm accepted regardless of throttle position (could arm at high throttle) | Added gate: arm only when throttle < `THROTTLE_IDLE_LIMIT` (1050µs) |
| P2 | Dead `PPMReceiver.h/.cpp` (project uses i-BUS, not PPM) + stale `dimag0g/PPM-reader` lib_dep | Deleted files, removed lib_dep from `platformio.ini` |
| P2 | `ledcSetup()` deprecation concern | Investigated: installed framework is arduino-esp32 **2.0.17** — old API is correct, no change needed |
| P3 | Magic numbers throughout `FlightController.cpp` (15 raw constants) | All moved to named `static constexpr` in `FlightController.h` |
| P3 | PID defaults defined separately in `FlightController`, `FlightControllerPID`, and `WebDashboardHandlers` | Single source: `kDefaultRate/Yaw/AngleKp/Ki/Kd` as public constants in `FlightController.h` |
| P3 | `3.14159f` in `MPU6500IMU.cpp` | Replaced with `kRadToDeg = 57.2957795f` constexpr |
| P3 | `400.0f` PID clamp hardcoded 8 times | Named `kOutputLimit = 400.0f` in `PIDController.h` |

---

## Known Gap

**No in-flight battery failsafe** — `FlightController::update()` never reads `battery_`. The battery drives only the LED blink in Battery Task. A low-voltage motor cutoff was intentionally removed (see earlier session) to prevent mid-air shutdown from transient voltage sag. If an audible/visual alarm plus a graceful land sequence is needed in the future, it would be a new feature, not a bug fix.

---

## Test Coverage

```
pio test -e native  →  4 test cases | 59 assertions | 0 failed
```

| Test file | What it covers |
|---|---|
| `test_pid.cpp` | P/I/D terms, anti-windup clamp, D-on-measurement, D-term LPF |
| `test_kalman.cpp` | Kalman predict/update, convergence |
| `test_simulation.cpp` | SimulatedHardware overrides and telemetry injection |
| `test_flight_controller.cpp` | Idle cutoff, arm-refused-on-high-throttle, motor mixing, motor min-clamping, PPM failsafe |

---

## Next Steps

1. **Flash**: `pio run -e esp32dev --target upload`
2. **First power-up**:
   - Transmitter on, AUX1 low (disarmed).
   - Connect to `ESP32_Drone_Config` → `http://192.168.4.1/`.
   - Receiver tab: verify all stick movements map correctly.
   - Motor Test tab: spin each motor 1050–1100µs, confirm rotation direction.
3. **Hover test**:
   - Arm (AUX1 high, throttle at minimum). Wi-Fi shuts down automatically.
   - Short 10–20s hover. Disarm.
   - Reconnect to Wi-Fi, fetch CSV log, paste to AI for PID recommendations.
4. **PID tuning starting point** (loaded from NVS on each arm):
   - Rate: Kp=0.7, Ki=0.0, Kd=0.01
   - Yaw: Kp=2.0, Ki=12.0, Kd=0.0
   - Angle: Kp=1.5, Kd=0.0 (increase Kd only after stable hover confirmed)
5. **Hardware note**: If upgrading to arduino-esp32 **3.0+**, migrate LEDC calls in `PWMESP32Motors.cpp`:
   - `ledcSetup(i, freq, bits)` + `ledcAttachPin(pin, i)` → `ledcAttach(pin, freq, bits)`
   - `ledcWrite(i, duty)` → `ledcWrite(pin, duty)`
