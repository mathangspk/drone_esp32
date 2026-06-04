#ifndef ADCBATTERYMONITOR_H
#define ADCBATTERYMONITOR_H

#include "interfaces/IBattery.h"
#include <atomic>

/**
 * @brief ESP32 analog ADC battery voltage monitor driver using a voltage divider.
 */
class ADCBatteryMonitor : public IBattery {
public:
    ADCBatteryMonitor(int analogPin, float refVoltage, float r1Value, float r2Value);

    void init();
    void update();          // reads ADC and advances EMA — call from one task only
    float readVoltage() const override; // pure getter, safe to call from any task
    bool isLow() const override;

    // Simulation/Override functionality
    void setOverride(float voltage) override;
    void setOverrideActive(bool active) override { overrideActive_ = active; }
    bool isOverrideActive() const override { return overrideActive_; }

private:
    static constexpr int   ADC_RESOLUTION_BITS = 12;
    static constexpr float ADC_MAX_VALUE       = (1 << ADC_RESOLUTION_BITS) - 1; // 4095
    static constexpr float EMA_ALPHA           = 0.05f; // ~1Hz cutoff at 250Hz sample rate
    static constexpr int   WARMUP_SAMPLES      = 20;

    int pin_;
    float refVoltage_;
    float r1_;
    float r2_;
    std::atomic<float> currentVoltage_{11.1f};

    // Override states
    bool overrideActive_ = false;
    float oVoltage_ = 11.1f;
};

#endif // ADCBATTERYMONITOR_H
