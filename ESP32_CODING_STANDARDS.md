# Bộ Quy Tắc Viết Code cho Dự Án ESP32

> **Mục đích:** Tài liệu chuẩn hóa cách viết firmware ESP32, dùng cho mọi dự án (IoT, mobile equipment, industrial controller). Có thể dùng trực tiếp làm `CLAUDE.md`, `.cursorrules`, hoặc `CONTRIBUTING.md`.
> **Phạm vi:** Ưu tiên ESP-IDF (FreeRTOS native). Phần nào khác với Arduino/PlatformIO sẽ ghi chú rõ `[Arduino]`.
> **Quy ước mức độ:** **MUST** (bắt buộc) · **SHOULD** (nên) · **MAY** (tùy chọn).

---

## 0. Triết Lý Chung

- **MUST** coi mỗi thiết bị là một hệ thống chạy 24/7 không người giám sát. Mọi lỗi đều phải tự phục hồi hoặc reset an toàn, không bao giờ treo im lặng.
- **MUST** thiết kế "fail-safe": khi mất WiFi/MQTT/cảm biến, thiết bị vẫn vận hành ở trạng thái an toàn, không gây hại phần cứng (đặc biệt với hệ điều khiển như inverter, van điện, motor).
- **MUST** ưu tiên non-blocking. Không có `delay()` chặn trong luồng chính.
- **SHOULD** tách biệt rõ 3 tầng: **HAL (phần cứng)** → **Logic nghiệp vụ** → **Giao tiếp (network/UI)**. Logic không được gọi trực tiếp hàm phần cứng.
- **SHOULD** mọi giá trị có thể thay đổi (chân GPIO, ngưỡng cảm biến, topic MQTT, timeout) đều là hằng số/cấu hình, không "magic number" rải rác trong code.

---

## 1. Cấu Trúc Dự Án & Build System

- **MUST** dùng **PlatformIO** hoặc **ESP-IDF (idf.py)** cho dự án nghiêm túc. Tránh dùng Arduino IDE thuần cho dự án production (khó quản lý lib, không có CI).
- **MUST** cố định version: ghi rõ version ESP-IDF / Arduino-ESP32 core và version từng thư viện trong `platformio.ini` hoặc `idf_component.yml`. Không dùng `latest`.
- **MUST** cấu trúc thư mục rõ ràng:

```
project/
├── platformio.ini / CMakeLists.txt
├── sdkconfig.defaults          # cấu hình ESP-IDF mặc định (commit vào git)
├── partitions.csv              # bảng phân vùng (định nghĩa rõ OTA, NVS)
├── include/                    # header public
├── src/
│   ├── main.cpp                # CHỈ khởi tạo + tạo task, không chứa logic
│   ├── hal/                    # driver phần cứng (sensor, actuator, bus)
│   ├── core/                   # logic nghiệp vụ
│   ├── net/                    # wifi, mqtt, ota, http
│   └── config/                 # config.h, secrets, pin map
├── lib/                        # thư viện nội bộ tách riêng
├── test/                       # unit test (chạy trên host)
└── docs/
```

- **MUST** đưa `secrets.h` / `secrets.ini` vào `.gitignore`. Commit file `secrets.example.h` để người khác biết cấu trúc.
- **SHOULD** mỗi module là một cặp `.h/.cpp` (hoặc `.c`) độc lập, có thể test riêng.

---

## 2. Quy Ước Đặt Tên

| Loại | Quy ước | Ví dụ |
|---|---|---|
| File | `snake_case` | `sensor_oxygen.cpp` |
| Class / struct / enum | `PascalCase` | `class OxygenSensor` |
| Hàm / biến | `camelCase` hoặc `snake_case` (chọn 1, nhất quán) | `readPpm()` |
| Hằng số / macro | `UPPER_SNAKE_CASE` | `MAX_RETRY_COUNT` |
| Biến thành viên private | hậu tố `_` | `uint16_t value_;` |
| Biến global (hạn chế dùng) | tiền tố `g_` | `g_systemState` |
| Task FreeRTOS | hậu tố `Task` | `sensorTask` |
| GPIO | tiền tố `PIN_` | `PIN_OXYGEN_ADC` |

- **MUST** đặt tên có ngữ nghĩa. `temp` mơ hồ (temperature hay temporary?) → dùng `tempC` / `tmpBuffer`.
- **MUST** đơn vị nằm trong tên hoặc comment: `delayMs`, `timeoutSec`, `voltageMv`, `pressureKpa`.

---

## 3. Tổ Chức Code & Kiến Trúc

- **MUST** `main.cpp` (hoặc `app_main`) chỉ làm: init hardware → init NVS/config → tạo task → trả về. Không viết logic ở đây.
- **MUST** tách driver phần cứng (HAL) khỏi logic. HAL trả về dữ liệu thô + mã lỗi; logic quyết định hành động.
- **SHOULD** dùng interface/abstract class cho cảm biến cùng loại để dễ thay thế và test (ví dụ `ISensor` với `read()` và `isHealthy()`).
- **MUST** mỗi hàm làm **một việc**. Hàm dài quá ~50 dòng là dấu hiệu cần tách.
- **MUST** kiểm tra giá trị trả về của **mọi** API ESP-IDF trả `esp_err_t`:

```c
esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return err;
}
// hoặc ESP_ERROR_CHECK(err) nếu lỗi này là không thể phục hồi
```

---

## 4. Quản Lý Bộ Nhớ (RAM rất hạn chế!)

- **MUST** ưu tiên cấp phát tĩnh (static / stack) thay vì `malloc`/`new` trong runtime. Tránh phân mảnh heap khi chạy lâu dài.
- **MUST** **KHÔNG** dùng `String` (Arduino) trong vòng lặp hoặc xử lý dữ liệu liên tục → gây phân mảnh heap nghiêm trọng. Dùng `char[]` cố định + `snprintf`.
- **MUST** kiểm tra heap định kỳ và log cảnh báo khi thấp: `esp_get_free_heap_size()`, `esp_get_minimum_free_heap_size()`.
- **MUST** với buffer/JSON lớn (HTTP, OTA), kiểm tra free heap trước khi cấp phát; ưu tiên dùng PSRAM nếu board có (`heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`).
- **MUST** mỗi `malloc`/`new` phải có `free`/`delete` tương ứng; ưu tiên RAII (smart pointer, object tự dọn trong destructor).
- **SHOULD** dùng `static` buffer cho JSON serialization (ArduinoJson: `StaticJsonDocument` thay vì `DynamicJsonDocument` khi kích thước biết trước).

---

## 5. FreeRTOS: Task, Priority, Core

- **MUST** kiến trúc theo nhiều task chuyên trách (sensor, network, control, logging) thay vì 1 vòng `loop()` khổng lồ.
- **MUST** đặt **stack size** đúng. Bắt đầu rộng rãi (ví dụ 4096 byte), đo bằng `uxTaskGetStackHighWaterMark()`, rồi siết lại với biên an toàn ~25%. Stack overflow là nguyên nhân crash phổ biến nhất.
- **MUST** gán priority có chủ đích. Task điều khiển an toàn/real-time > network > logging. Đừng để mọi task cùng priority.
- **MUST** ghim core đúng mục đích: ESP32 có 2 core. Core 0 thường chạy WiFi/BT stack; ghim task tính toán nặng/real-time lên Core 1 (`xTaskCreatePinnedToCore`).
- **MUST** **không bao giờ** dùng `delay()` busy-loop trong task; dùng `vTaskDelay(pdMS_TO_TICKS(ms))` để nhường CPU.
- **SHOULD** task chạy theo chu kỳ chính xác → dùng `vTaskDelayUntil()` thay vì `vTaskDelay()` để tránh trôi thời gian.

```c
void sensorTask(void *pvParameters) {
    const TickType_t period = pdMS_TO_TICKS(1000);
    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        readAndPublishSensor();
        vTaskDelayUntil(&lastWake, period);   // chu kỳ ổn định 1s
    }
}
```

---

## 6. Đồng Bộ Hóa & Giao Tiếp Giữa Task

- **MUST** **KHÔNG** chia sẻ biến giữa các task mà không bảo vệ. Dùng **Queue** để truyền dữ liệu, **Mutex** để bảo vệ tài nguyên dùng chung (bus I2C/SPI, biến trạng thái).
- **MUST** ưu tiên **Queue** hơn biến global cho luồng dữ liệu (sensor → control → network). Decoupling tốt hơn, không cần lock thủ công.
- **MUST** dùng **Mutex** (không phải binary semaphore) khi bảo vệ tài nguyên, để hưởng cơ chế priority inheritance, tránh priority inversion.
- **MUST** mọi `xSemaphoreTake` phải có timeout (không dùng `portMAX_DELAY` vô hạn ở luồng quan trọng) và xử lý trường hợp lấy lock thất bại.
- **MUST** truy cập biến chia sẻ đơn giản giữa task và ISR → khai báo `volatile` và dùng kiểu atomic / critical section (`portENTER_CRITICAL`).
- **SHOULD** dùng **Event Group** để chờ nhiều điều kiện (ví dụ: WiFi connected AND time synced AND NVS loaded).

---

## 7. Interrupt (ISR) — Cực Kỳ Nghiêm Ngặt

- **MUST** ISR phải **cực ngắn và nhanh**. Chỉ set cờ, đẩy queue, hoặc give semaphore. Mọi xử lý nặng đưa ra task qua "deferred processing".
- **MUST** **KHÔNG** gọi trong ISR: `printf`/`ESP_LOGx`, `malloc`, `delay`, hàm blocking, hay hàm float (trừ khi đã cấu hình).
- **MUST** trong ISR chỉ dùng API có hậu tố `FromISR` (`xQueueSendFromISR`, `xSemaphoreGiveFromISR`).
- **MUST** hàm ISR (và mọi hàm nó gọi) đặt trong IRAM: `IRAM_ATTR void IRAM_ATTR myIsr()`.
- **MUST** xử lý `portYIELD_FROM_ISR(higherPriorityTaskWoken)` để đánh thức task đúng lúc.

```c
void IRAM_ATTR gpioIsr(void *arg) {
    BaseType_t hpw = pdFALSE;
    xSemaphoreGiveFromISR(g_pulseSem, &hpw);
    portYIELD_FROM_ISR(hpw);
}
```

---

## 8. Timer & Quy Tắc Non-Blocking

- **MUST** **CẤM** `delay()`/`vTaskDelay()` để chờ trong logic nghiệp vụ chính. Dùng máy trạng thái (state machine) + mốc thời gian (`millis()` / `esp_timer_get_time()`).
- **MUST** debounce nút bấm bằng thời gian, không bằng `delay()`.
- **SHOULD** dùng `esp_timer` (high-resolution) cho việc định thời, hoặc FreeRTOS software timer cho callback định kỳ nhẹ.
- **SHOULD** mọi thao tác chờ I/O (đọc cảm biến chậm, HTTP) đều có **timeout** rõ ràng.

---

## 9. Watchdog & Xử Lý Lỗi

- **MUST** bật **Task Watchdog Timer (TWDT)** và feed nó định kỳ trong các task quan trọng. Task treo → reset tự động.
- **MUST** không tắt watchdog để "cho dễ". Nếu task cần chạy lâu, chia nhỏ và feed watchdog đúng cách.
- **MUST** mọi vòng `for(;;)` trong task phải có điểm nhường CPU (`vTaskDelay`) để không đói watchdog/IDLE task.
- **MUST** log lý do reset khi khởi động: `esp_reset_reason()` → giúp chẩn đoán brownout, panic, watchdog.
- **SHOULD** lưu lại crash/coredump (`esp_core_dump`) vào flash để phân tích sự cố ngoài hiện trường.
- **MUST** xử lý brownout: cấp nguồn đủ dòng (đặc biệt khi WiFi TX peak ~500mA), tụ lọc đủ lớn. Đây là nguyên nhân reset ngẫu nhiên rất hay gặp.

---

## 10. GPIO & Ngoại Vi (I2C / SPI / UART / ADC)

- **MUST** định nghĩa toàn bộ pin map trong **một file duy nhất** (`config/pins.h`), không hardcode số chân rải rác.
- **MUST** tránh các chân nhạy cảm khi boot (strapping pins: GPIO0, 2, 12, 15...) cho thiết bị ngoại vi nếu không cần.
- **MUST** ADC của ESP32 phi tuyến → **MUST** dùng `esp_adc_cal` (calibration) khi cần độ chính xác (ví dụ đọc cảm biến oxy/áp suất analog). Không tin tưởng giá trị raw.
- **MUST** bus I2C/SPI dùng chung nhiều thiết bị → bảo vệ bằng Mutex khi truy cập từ nhiều task.
- **MUST** mọi giao tiếp bus có timeout và kiểm tra mã lỗi; thiết bị không phản hồi → đánh dấu "unhealthy", không treo task.
- **SHOULD** với cảm biến quan trọng, đọc nhiều mẫu + lọc (median/moving average) để chống nhiễu.

---

## 11. Networking (WiFi / BLE / MQTT)

- **MUST** quản lý WiFi/MQTT bằng máy trạng thái + auto-reconnect có backoff (1s, 2s, 4s... có giới hạn). Không spam reconnect liên tục.
- **MUST** thiết bị phải hoạt động được khi **mất mạng**: vẫn điều khiển an toàn, buffer dữ liệu (vòng đệm) và gửi lại khi có mạng.
- **MUST** MQTT: dùng **LWT (Last Will & Testament)** để báo offline; chọn QoS phù hợp (QoS1 cho lệnh điều khiển quan trọng).
- **MUST** đặt tên topic theo chuẩn phân cấp nhất quán, gồm device ID: `org/site/{deviceId}/telemetry`, `.../cmd`, `.../status`.
- **MUST** payload dùng JSON có schema rõ ràng, kèm `timestamp` và `deviceId`. Validate input lệnh trước khi thực thi (đặc biệt lệnh điều khiển phần cứng).
- **MUST** dùng **TLS** cho MQTT/HTTP ở production. Không gửi credential/dữ liệu cảm biến qua kết nối thuần.
- **SHOULD** tách credential WiFi/MQTT ra NVS hoặc provisioning, không hardcode trong firmware.

---

## 12. Lưu Trữ (NVS / SPIFFS / LittleFS)

- **MUST** dùng **NVS** cho cấu hình/key-value nhỏ (WiFi cred, calibration, counter). Đừng dùng cho dữ liệu lớn/ghi liên tục.
- **MUST** xử lý `ESP_ERR_NVS_NO_FREE_PAGES` (gọi `nvs_flash_erase` + init lại) ở bước khởi tạo.
- **MUST** dùng **LittleFS** thay SPIFFS (SPIFFS đã lỗi thời, không hỗ trợ thư mục, dễ hỏng).
- **MUST** flash có giới hạn ghi (~10⁴–10⁵ chu kỳ). **KHÔNG** ghi NVS/flash trong vòng lặp tốc độ cao. Gom dữ liệu, ghi định kỳ hoặc khi có thay đổi.
- **MUST** định nghĩa `partitions.csv` rõ ràng (đủ chỗ cho 2 phân vùng OTA + NVS + storage).

---

## 13. OTA (Cập Nhật Firmware Từ Xa)

- **MUST** thiết kế OTA an toàn ngay từ đầu: 2 phân vùng app + **rollback tự động** khi firmware mới không "mark valid".
- **MUST** xác thực firmware: kiểm tra version, checksum/signature trước khi áp dụng.
- **MUST** không cho phép OTA khi thiết bị đang ở trạng thái nguy hiểm (đang điều khiển motor/van). Chỉ update ở trạng thái an toàn.
- **SHOULD** dùng HTTPS cho tải firmware; có cơ chế báo tiến trình và kết quả OTA về server.

---

## 14. Quản Lý Năng Lượng (Power)

- **MUST** với thiết bị chạy pin: dùng **deep sleep** + RTC wakeup; lưu trạng thái cần giữ vào RTC memory (`RTC_DATA_ATTR`).
- **SHOULD** tắt/giảm WiFi BT khi không dùng; dùng modem-sleep, DTIM.
- **SHOULD** đo dòng tiêu thụ thực tế từng trạng thái, đừng đoán.

---

## 15. Bảo Mật

- **MUST** **KHÔNG** hardcode mật khẩu/API key/private key trong source. Dùng NVS, provisioning, hoặc inject lúc build.
- **MUST** với production thật: bật **Flash Encryption** + **Secure Boot** (cân nhắc vì không thể đảo ngược — đọc kỹ tài liệu trước khi bật trên hàng loạt).
- **MUST** validate mọi input từ ngoài (MQTT cmd, HTTP, BLE) trước khi tác động phần cứng. Giả định mọi input là độc hại.
- **MUST** dùng TLS với chứng chỉ được verify (gắn CA cert), không bỏ qua xác thực chứng chỉ.
- **SHOULD** vô hiệu hóa/khóa JTAG và cổng debug trên thiết bị xuất xưởng.

---

## 16. Logging & Telemetry

- **MUST** dùng `ESP_LOGx` với **TAG** riêng cho mỗi module và đúng level (`E/W/I/D/V`). Không dùng `printf` lung tung.

```c
static const char *TAG = "OXY_SENSOR";
ESP_LOGI(TAG, "ppm=%u, healthy=%d", ppm, healthy);
```

- **MUST** log level điều chỉnh được lúc build/runtime; production để mức `INFO`/`WARN`, debug mới bật `DEBUG`.
- **MUST** **KHÔNG** log trong ISR hoặc trong vòng lặp nóng.
- **MUST** không log dữ liệu nhạy cảm (mật khẩu, token).
- **SHOULD** telemetry (số liệu vận hành: heap, RSSI, uptime, reset reason) gửi định kỳ về server để giám sát đội thiết bị từ xa.

---

## 17. Cấu Hình & Secrets

- **MUST** mọi tham số có thể thay đổi gom vào `config.h` / `sdkconfig` / NVS, không rải rác.
- **MUST** phân biệt 3 nguồn cấu hình: **build-time** (Kconfig/macro), **provisioned** (NVS, không đổi sau xuất xưởng), **runtime** (đổi qua MQTT/UI).
- **MUST** có cấu hình mặc định an toàn để thiết bị vẫn khởi động được khi NVS trống/hỏng.

---

## 18. Testing

- **MUST** tách logic nghiệp vụ khỏi phần cứng để **unit test trên host** (PC) bằng Unity/GoogleTest — chạy nhanh, không cần nạp board.
- **SHOULD** mock HAL (sensor/actuator) để test logic điều khiển và xử lý lỗi.
- **SHOULD** có ít nhất một bài test tích hợp trên phần cứng thật (hardware-in-the-loop) cho luồng quan trọng.
- **MUST** test các tình huống lỗi: mất WiFi giữa chừng, cảm biến trả giá trị bất thường, mất nguồn khi đang ghi NVS.

---

## 19. Documentation & Comments

- **MUST** comment giải thích **TẠI SAO**, không phải **CÁI GÌ** (code đã nói code làm gì).
- **MUST** mỗi module có comment đầu file: chức năng, phụ thuộc phần cứng, chân sử dụng.
- **MUST** ghi rõ giả định và ràng buộc nguy hiểm (ví dụ: "Hàm này phải gọi sau khi NVS init", "Không gọi từ ISR").
- **SHOULD** `README.md` mô tả: sơ đồ kiến trúc, pin map, cách build/flash, sơ đồ các task và quan hệ.

---

## 20. Version Control & CI/CD

- **MUST** `.gitignore` đúng: bỏ `.pio/`, `build/`, `sdkconfig` (giữ `sdkconfig.defaults`), `secrets.*`.
- **MUST** commit message rõ ràng; mỗi commit một thay đổi logic.
- **MUST** firmware có **version** (semantic versioning) nhúng trong build và báo cáo qua telemetry/OTA.
- **SHOULD** CI tự động: build cho mọi PR, chạy unit test trên host, kiểm tra format (`clang-format`).
- **SHOULD** tag git mỗi bản release tương ứng với firmware đã deploy thực tế.

---

## 21. Checklist Trước Khi Release (Production)

- [ ] Stack high-water-mark của mọi task có biên an toàn ≥ 20–25%
- [ ] Heap ổn định sau khi chạy nhiều giờ (không rò rỉ, không phân mảnh tăng dần)
- [ ] Task Watchdog bật; test thử treo task → thiết bị reset đúng
- [ ] Auto-reconnect WiFi/MQTT hoạt động (test rút mạng, tắt router)
- [ ] Thiết bị an toàn khi mất mạng và khi mất nguồn đột ngột
- [ ] OTA + rollback đã test thực tế
- [ ] Không còn `delay()` blocking, `String` trong vòng lặp, hay log trong ISR
- [ ] Không hardcode secret; TLS bật; input được validate
- [ ] `esp_reset_reason()` được log; coredump bật
- [ ] Nguồn cấp đủ dòng, không brownout khi WiFi TX peak
- [ ] Calibration ADC/cảm biến đã thực hiện
- [ ] Version firmware đúng và báo cáo được qua telemetry

---

*Tài liệu sống — cập nhật khi rút ra bài học mới từ thực tế triển khai thiết bị.*
