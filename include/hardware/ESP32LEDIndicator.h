#ifndef ESP32LEDINDICATOR_H
#define ESP32LEDINDICATOR_H

#include "interfaces/IStatusIndicator.h"
#include <stdint.h>

/**
 * @brief Concrete ESP32 driver class for status LED indicator on a GPIO pin.
 * Handles non-blocking blink patterns based on flight state and battery levels.
 */
class ESP32LEDIndicator : public IStatusIndicator {
public:
    explicit ESP32LEDIndicator(uint8_t pin);
    
    void init() override;
    void setLowBattery(bool isLow) override;
    void setArmed(bool isArmed) override;
    void update() override;

private:
    uint8_t pin_;
    bool isLowBattery_ = false;
    bool isArmed_ = false;
    
    uint32_t lastToggleTime_ = 0;
    bool ledState_ = false;
};

#endif // ESP32LEDINDICATOR_H
