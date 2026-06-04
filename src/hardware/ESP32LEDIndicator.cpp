#include "hardware/ESP32LEDIndicator.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

ESP32LEDIndicator::ESP32LEDIndicator(uint8_t pin) : pin_(pin) {}

void ESP32LEDIndicator::init() {
#ifndef NATIVE_BUILD
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
#endif
}

void ESP32LEDIndicator::setLowBattery(bool isLow) {
    isLowBattery_ = isLow;
}

void ESP32LEDIndicator::setArmed(bool isArmed) {
    isArmed_ = isArmed;
}

void ESP32LEDIndicator::update() {
#ifndef NATIVE_BUILD
    uint32_t now = millis();
    if (isLowBattery_) {
        // Rapid warning blink (100ms ON, 100ms OFF)
        if (now - lastToggleTime_ >= 100) {
            ledState_ = !ledState_;
            digitalWrite(pin_, ledState_ ? HIGH : LOW);
            lastToggleTime_ = now;
        }
    } else if (isArmed_) {
        // Armed state: Solid ON
        if (!ledState_) {
            ledState_ = true;
            digitalWrite(pin_, HIGH);
        }
    } else {
        // Disarmed / Config state: Heartbeat (50ms pulse every 1000ms)
        if (ledState_) {
            if (now - lastToggleTime_ >= 50) {
                ledState_ = false;
                digitalWrite(pin_, LOW);
                lastToggleTime_ = now;
            }
        } else {
            if (now - lastToggleTime_ >= 950) {
                ledState_ = true;
                digitalWrite(pin_, HIGH);
                lastToggleTime_ = now;
            }
        }
    }
#endif
}
