#ifndef ADCBATTERYMONITOR_H
#define ADCBATTERYMONITOR_H

#include "interfaces/IBattery.h"

/**
 * @brief ESP32 analog ADC battery voltage monitor driver using a voltage divider.
 */
class ADCBatteryMonitor : public IBattery {
public:
    ADCBatteryMonitor(int analogPin, float refVoltage, float r1Value, float r2Value);

    void init();
    float readVoltage() const override;
    bool isLow() const override;

    // Simulation/Override functionality
    void setOverride(float voltage) override;
    void setOverrideActive(bool active) override { overrideActive_ = active; }
    bool isOverrideActive() const override { return overrideActive_; }

private:
    int pin_;
    float refVoltage_;
    float r1_;
    float r2_;
    mutable float currentVoltage_ = 11.1f;

    // Override states
    bool overrideActive_ = false;
    float oVoltage_ = 11.1f;
};

#endif // ADCBATTERYMONITOR_H
