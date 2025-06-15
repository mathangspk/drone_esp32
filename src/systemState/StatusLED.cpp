#include "systemState/StatusLED.h"

StatusLED::StatusLED(uint8_t pin) : _pin(pin), _state(false) {}

void StatusLED::begin()
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void StatusLED::on()
{
    _state = true;
    digitalWrite(_pin, HIGH);
}

void StatusLED::off()
{
    _state = false;
    digitalWrite(_pin, LOW);
}

void StatusLED::toggle()
{
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
}

void StatusLED::blink(uint16_t duration_ms, uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        on();
        delay(duration_ms);
        off();
        delay(duration_ms);
    }
}
// Nháy 5Hz cho operator mode
void StatusLED::blinkOperator()
{
    _interval = 100; // 5Hz = 100ms
}

// Nháy 1Hz cho config mode
void StatusLED::blinkConfig()
{
    _interval = 500; // 1Hz = 500ms
}

// Gọi hàm này trong vòng lặp để cập nhật trạng thái LED
void StatusLED::update()
{
    unsigned long now = millis();
    if (now - _lastToggle >= _interval)
    {
        toggle();
        _lastToggle = now;
    }
}