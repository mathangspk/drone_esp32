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

private:
    uint8_t _pin;
    bool _state;
};

#endif
