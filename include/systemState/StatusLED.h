#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

class StatusLED
{
public:
    StatusLED(uint8_t pin);
    void begin();
    void on();
    void off();
    void blink(uint16_t duration_ms, uint8_t times);
    void toggle();
    // Thêm các hàm nháy chuyên biệt
    void blinkOperator(); // 5Hz
    void blinkConfig();   // 1Hz

    void update(); // Gọi trong vòng lặp để cập nhật trạng thái nháy

private:
    uint8_t _pin;
    bool _state;
    unsigned long _lastToggle = 0;
    uint16_t _interval = 500; // ms
};

#endif
