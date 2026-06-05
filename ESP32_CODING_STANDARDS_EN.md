# ESP32 Project Coding Standards

> **Purpose:** A standardized document for writing ESP32 firmware, applicable to all projects (IoT, mobile equipment, industrial controllers). Can be used directly as `CLAUDE.md`, `.cursorrules`, or `CONTRIBUTING.md`.
> **Scope:** Prioritizes ESP-IDF (FreeRTOS native). Any sections differing for Arduino/PlatformIO will be explicitly marked `[Arduino]`.
> **Convention Levels:** **MUST** (mandatory) · **SHOULD** (recommended) · **MAY** (optional).

---

## 0. General Philosophy

- **MUST** treat every device as an unattended 24/7 system. All errors must self-recover or trigger a safe reset; never fail silently or hang.
- **MUST** design for "fail-safe": upon losing WiFi/MQTT/sensors, the device must operate in a safe state, preventing hardware damage (especially for control systems like inverters, electric valves, motors).
- **MUST** prioritize non-blocking operations. No blocking `delay()` in the main thread.
- **SHOULD** clearly separate 3 layers: **HAL (Hardware)** → **Business Logic** → **Communication (Network/UI)**. Logic must not directly invoke hardware functions.
- **SHOULD** make all mutable values (GPIO pins, sensor thresholds, MQTT topics, timeouts) constants/configurations; no "magic numbers" scattered in the code.

---

## 1. Project Structure & Build System

- **MUST** use **PlatformIO** or **ESP-IDF (idf.py)** for serious projects. Avoid raw Arduino IDE for production (hard to manage libs, no CI).
- **MUST** pin versions: explicitly state ESP-IDF / Arduino-ESP32 core versions and individual library versions in `platformio.ini` or `idf_component.yml`. Never use `latest`.
- **MUST** have a clear directory structure:

```
project/
├── platformio.ini / CMakeLists.txt
├── sdkconfig.defaults          # default ESP-IDF configs (commit to git)
├── partitions.csv              # partition table (define OTA, NVS clearly)
├── include/                    # public headers
├── src/
│   ├── main.cpp                # ONLY for initialization + task creation, NO logic
│   ├── hal/                    # hardware drivers (sensor, actuator, bus)
│   ├── core/                   # business logic
│   ├── net/                    # wifi, mqtt, ota, http
│   └── config/                 # config.h, secrets, pin maps
├── lib/                        # isolated internal libraries
├── test/                       # unit tests (run on host)
└── docs/
```

- **MUST** add `secrets.h` / `secrets.ini` to `.gitignore`. Commit a `secrets.example.h` so others know the structure.
- **SHOULD** ensure each module is an independent `.h/.cpp` (or `.c`) pair that can be tested separately.

---

## 2. Naming Conventions

| Type | Convention | Example |
|---|---|---|
| File | `snake_case` | `sensor_oxygen.cpp` |
| Class / struct / enum | `PascalCase` | `class OxygenSensor` |
| Function / variable | `camelCase` or `snake_case` (pick one, consistent) | `readPpm()` |
| Constant / macro | `UPPER_SNAKE_CASE` | `MAX_RETRY_COUNT` |
| Private member variable | `_` suffix | `uint16_t value_;` |
| Global variable (limit use) | `g_` prefix | `g_systemState` |
| FreeRTOS Task | `Task` suffix | `sensorTask` |
| GPIO | `PIN_` prefix | `PIN_OXYGEN_ADC` |

- **MUST** use semantic names. `temp` is ambiguous (temperature or temporary?) → use `tempC` / `tmpBuffer`.
- **MUST** include units in the name or comment: `delayMs`, `timeoutSec`, `voltageMv`, `pressureKpa`.

---

## 3. Code Organization & Architecture

- **MUST** ensure `main.cpp` (or `app_main`) only handles: init hardware → init NVS/config → create tasks → return. Do not write business logic here.
- **MUST** decouple hardware drivers (HAL) from logic. HAL returns raw data + error codes; logic decides the action.
- **SHOULD** use interfaces/abstract classes for similar sensors to easily swap and test (e.g., `ISensor` with `read()` and `isHealthy()`).
- **MUST** make sure each function does **one thing**. A function over ~50 lines is a sign it needs splitting.
- **MUST** check the return value of **every** ESP-IDF API that returns `esp_err_t`:

```c
esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return err;
}
// or ESP_ERROR_CHECK(err) if this error is unrecoverable
```

---

## 4. Memory Management (RAM is very limited!)

- **MUST** prioritize static allocation (static / stack) over runtime `malloc`/`new`. Prevent heap fragmentation over long uptimes.
- **MUST NOT** use `String` (Arduino) in loops or continuous data processing → causes severe heap fragmentation. Use fixed `char[]` + `snprintf`.
- **MUST** periodically check the heap and log a warning when low: `esp_get_free_heap_size()`, `esp_get_minimum_free_heap_size()`.
- **MUST** check free heap before allocating large buffers/JSONs (HTTP, OTA); prioritize PSRAM if the board has it (`heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`).
- **MUST** match every `malloc`/`new` with a `free`/`delete`; prioritize RAII (smart pointers, auto-cleanup in destructors).
- **SHOULD** use `static` buffers for JSON serialization (ArduinoJson: `StaticJsonDocument` instead of `DynamicJsonDocument` when size is known).

---

## 5. FreeRTOS: Tasks, Priorities, Cores

- **MUST** architect using multiple specialized tasks (sensor, network, control, logging) instead of one giant `loop()`.
- **MUST** set **stack size** correctly. Start generous (e.g., 4096 bytes), measure using `uxTaskGetStackHighWaterMark()`, then tighten it with a ~25% safety margin. Stack overflow is the most common cause of crashes.
- **MUST** assign priorities intentionally. Safety/real-time control task > network > logging. Do not put all tasks at the same priority.
- **MUST** pin cores correctly: ESP32 has 2 cores. Core 0 usually runs the WiFi/BT stack; pin heavy computation/real-time tasks to Core 1 (`xTaskCreatePinnedToCore`).
- **MUST NEVER** use busy-loop `delay()` inside a task; use `vTaskDelay(pdMS_TO_TICKS(ms))` to yield the CPU.
- **SHOULD** use `vTaskDelayUntil()` instead of `vTaskDelay()` for tasks needing precise periodicity to prevent time drift.

```c
void sensorTask(void *pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(1000);
    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        readAndPublishSensor();
        vTaskDelayUntil(&lastWake, period);   // stable 1s period
    }
}
```

---

## 6. Synchronization & Inter-Task Communication

- **MUST NOT** share variables between tasks without protection. Use **Queues** to pass data, **Mutexes** to protect shared resources (I2C/SPI buses, state variables).
- **MUST** prioritize **Queues** over global variables for data flow (sensor → control → network). Decouples better, avoids manual locking.
- **MUST** use **Mutexes** (not binary semaphores) when protecting resources to benefit from priority inheritance and avoid priority inversion.
- **MUST** include a timeout for every `xSemaphoreTake` (do not use infinite `portMAX_DELAY` in critical threads) and handle lock failure cases.
- **MUST** declare simple shared variables accessed between tasks and ISRs as `volatile` and use atomic types / critical sections (`portENTER_CRITICAL`).
- **SHOULD** use **Event Groups** to wait for multiple conditions (e.g., WiFi connected AND time synced AND NVS loaded).

---

## 7. Interrupts (ISR) — Extremely Strict

- **MUST** keep ISRs **extremely short and fast**. Only set flags, push to queues, or give semaphores. Offload all heavy processing to a task via "deferred processing".
- **MUST NOT** call inside an ISR: `printf`/`ESP_LOGx`, `malloc`, `delay`, blocking functions, or float operations (unless specifically configured).
- **MUST** only use APIs with the `FromISR` suffix inside an ISR (`xQueueSendFromISR`, `xSemaphoreGiveFromISR`).
- **MUST** place ISR functions (and any functions they call) in IRAM: `IRAM_ATTR void IRAM_ATTR myIsr()`.
- **MUST** handle `portYIELD_FROM_ISR(higherPriorityTaskWoken)` to wake the appropriate task in time.

```c
void IRAM_ATTR gpioIsr(void *arg) {
    BaseType_t hpw = pdFALSE;
    xSemaphoreGiveFromISR(g_pulseSem, &hpw);
    portYIELD_FROM_ISR(hpw);
}
```

---

## 8. Timers & Non-Blocking Rules

- **MUST FORBID** `delay()`/`vTaskDelay()` for waiting in main business logic. Use state machines + timestamps (`millis()` / `esp_timer_get_time()`).
- **MUST** debounce buttons using time intervals, not `delay()`.
- **SHOULD** use `esp_timer` (high-resolution) for precise timing, or FreeRTOS software timers for lightweight periodic callbacks.
- **SHOULD** ensure every I/O wait operation (slow sensor read, HTTP) has a clear **timeout**.

---

## 9. Watchdogs & Error Handling

- **MUST** enable the **Task Watchdog Timer (TWDT)** and feed it periodically in critical tasks. Hanging tasks → automatic reset.
- **MUST NOT** disable the watchdog "for convenience". If a task needs to run long, break it down and feed the watchdog properly.
- **MUST** ensure every `for(;;)` loop in a task has a CPU yield point (`vTaskDelay`) to avoid starving the watchdog/IDLE task.
- **MUST** log the reset reason upon boot: `esp_reset_reason()` → helps diagnose brownouts, panics, watchdogs.
- **SHOULD** save crashes/coredumps (`esp_core_dump`) to flash for field failure analysis.
- **MUST** handle brownouts: provide sufficient current (especially during WiFi TX peaks ~500mA), use large enough filter capacitors. This is a very common cause of random resets.

---

## 10. GPIO & Peripherals (I2C / SPI / UART / ADC)

- **MUST** define the entire pin map in a **single file** (`config/pins.h`); no hardcoding pin numbers randomly.
- **MUST** avoid using sensitive strapping pins (GPIO0, 2, 12, 15...) for peripherals unless necessary.
- **MUST** calibrate the ESP32 ADC (it is non-linear) using `esp_adc_cal` when precision is required (e.g., reading analog oxygen/pressure sensors). Do not blindly trust raw values.
- **MUST** protect shared I2C/SPI buses with a Mutex when accessed from multiple tasks.
- **MUST** include timeouts and check error codes for all bus communications; if a device doesn't respond → mark as "unhealthy", do not hang the task.
- **SHOULD** read multiple samples + filter (median/moving average) for critical sensors to prevent noise.

---

## 11. Networking (WiFi / BLE / MQTT)

- **MUST** manage WiFi/MQTT using a state machine + auto-reconnect with backoff (1s, 2s, 4s... with a cap). Do not spam reconnects.
- **MUST** ensure the device operates when **offline**: continue safe control, buffer data, and resend when the network recovers.
- **MUST** use **LWT (Last Will & Testament)** for MQTT to signal offline status; choose appropriate QoS (QoS1 for critical commands).
- **MUST** use a consistent hierarchical topic structure, including the device ID: `org/site/{deviceId}/telemetry`, `.../cmd`, `.../status`.
- **MUST** format payloads as JSON with a clear schema, including a `timestamp` and `deviceId`. Validate input commands before execution (especially hardware control commands).
- **MUST** use **TLS** for MQTT/HTTP in production. Never send credentials/sensor data over plain connections.
- **SHOULD** separate WiFi/MQTT credentials into NVS or provisioning; never hardcode them in firmware.

---

## 12. Storage (NVS / SPIFFS / LittleFS)

- **MUST** use **NVS** for configurations/small key-value pairs (WiFi creds, calibrations, counters). Do not use for large data or continuous logging.
- **MUST** handle `ESP_ERR_NVS_NO_FREE_PAGES` (call `nvs_flash_erase` + re-init) during initialization.
- **MUST** use **LittleFS** over SPIFFS (SPIFFS is deprecated, doesn't support directories, and corrupts easily).
- **MUST** respect flash write limits (~10⁴–10⁵ cycles). **DO NOT** write to NVS/flash in high-speed loops. Batch data and write periodically or only on change.
- **MUST** clearly define `partitions.csv` (leaving enough room for 2 OTA partitions + NVS + storage).

---

## 13. OTA (Over-The-Air Updates)

- **MUST** design safe OTA from the start: 2 app partitions + **automatic rollback** if the new firmware fails to "mark valid".
- **MUST** authenticate firmware: check version, checksum/signature before applying.
- **MUST NOT** allow OTA when the device is in a hazardous state (actively controlling motors/valves). Only update in a safe state.
- **SHOULD** use HTTPS for downloading firmware; report OTA progress and results back to the server.

---

## 14. Power Management

- **MUST** use **deep sleep** + RTC wakeup for battery-operated devices; store necessary state in RTC memory (`RTC_DATA_ATTR`).
- **SHOULD** turn off/down WiFi/BT when not in use; utilize modem-sleep and DTIM.
- **SHOULD** measure actual power consumption for each state instead of guessing.

---

## 15. Security

- **MUST NOT** hardcode passwords/API keys/private keys in the source. Use NVS, provisioning, or inject during build time.
- **MUST** enable **Flash Encryption** + **Secure Boot** for real production runs (consider carefully as this is irreversible — read docs before mass enabling).
- **MUST** validate all external inputs (MQTT cmd, HTTP, BLE) before hardware actuation. Assume all input is malicious.
- **MUST** use TLS with verified certificates (bundled CA cert), do not bypass certificate validation.
- **SHOULD** disable/lock JTAG and debug ports on production units.

---

## 16. Logging & Telemetry

- **MUST** use `ESP_LOGx` with a distinct **TAG** for each module and the correct level (`E/W/I/D/V`). No random `printf`s.

```c
static const char *TAG = "OXY_SENSOR";
ESP_LOGI(TAG, "ppm=%u, healthy=%d", ppm, healthy);
```

- **MUST** make log levels configurable at build/runtime; production should be `INFO`/`WARN`, only enable `DEBUG` when troubleshooting.
- **MUST NOT** log inside an ISR or in tight, hot loops.
- **MUST NOT** log sensitive data (passwords, tokens).
- **SHOULD** send telemetry (operational metrics: heap, RSSI, uptime, reset reason) to the server periodically for remote fleet monitoring.

---

## 17. Configuration & Secrets

- **MUST** consolidate all mutable parameters into `config.h` / `sdkconfig` / NVS; avoid scattering them.
- **MUST** distinguish between 3 config sources: **build-time** (Kconfig/macros), **provisioned** (NVS, immutable post-factory), **runtime** (mutable via MQTT/UI).
- **MUST** provide a safe default configuration so the device can still boot if NVS is empty/corrupted.

---

## 18. Testing

- **MUST** separate business logic from hardware to run **host-based unit tests** (PC) using Unity/GoogleTest — fast execution, no board flashing required.
- **SHOULD** mock HAL (sensors/actuators) to test control logic and error handling.
- **SHOULD** include at least one Hardware-In-The-Loop integration test for critical flows.
- **MUST** test failure scenarios: dropping WiFi mid-operation, sensor returning abnormal values, power loss during NVS write.

---

## 19. Documentation & Comments

- **MUST** comment to explain **WHY**, not **WHAT** (the code already says what it does).
- **MUST** provide a file-level header comment for each module outlining its function, hardware dependencies, and used pins.
- **MUST** explicitly state assumptions and dangerous constraints (e.g., "This function must be called after NVS init", "Do not call from ISR").
- **SHOULD** maintain a `README.md` describing: architecture diagram, pin map, build/flash instructions, task layout and relations.

---

## 20. Version Control & CI/CD

- **MUST** maintain a proper `.gitignore`: exclude `.pio/`, `build/`, `sdkconfig` (keep `sdkconfig.defaults`), `secrets.*`.
- **MUST** write clear commit messages; one logical change per commit.
- **MUST** embed a semantic **version** into the firmware build and report it via telemetry/OTA.
- **SHOULD** setup automated CI: build for every PR, run host unit tests, check formatting (`clang-format`).
- **SHOULD** tag the git repo for every release that corresponds to deployed firmware.

---

## 21. Pre-Release Checklist (Production)

- [ ] High-water mark for all task stacks has a safety margin ≥ 20–25%
- [ ] Heap is stable after hours of running (no leaks, no continuous fragmentation)
- [ ] Task Watchdog is enabled; tested by freezing a task → device resets properly
- [ ] WiFi/MQTT auto-reconnect works (tested by unplugging network, powering off router)
- [ ] Device is safe during network loss and sudden power loss
- [ ] OTA + rollback tested in a real scenario
- [ ] No blocking `delay()`, `String` objects in loops, or logging in ISRs
- [ ] No hardcoded secrets; TLS enabled; input validated
- [ ] `esp_reset_reason()` is logged; coredump enabled
- [ ] Power supply provides sufficient current; no brownouts during WiFi TX peaks
- [ ] ADC/sensor calibrations applied
- [ ] Firmware version is correct and reported via telemetry

---
*A living document — update as new lessons are learned from actual device deployments.*
