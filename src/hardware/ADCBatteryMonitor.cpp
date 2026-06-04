#include "hardware/ADCBatteryMonitor.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

ADCBatteryMonitor::ADCBatteryMonitor(int analogPin, float refVoltage, float r1Value, float r2Value)
    : pin_(analogPin), refVoltage_(refVoltage), r1_(r1Value), r2_(r2Value) {}

void ADCBatteryMonitor::init() {
#ifndef NATIVE_BUILD
    analogReadResolution(ADC_RESOLUTION_BITS);
    pinMode(pin_, INPUT);

    // Warm up the filter to establish a correct baseline immediately on boot
    float sumVoltage = 0.0f;
    for (int i = 0; i < WARMUP_SAMPLES; ++i) {
        int rawValue = analogRead(pin_);
        float voltage = (static_cast<float>(rawValue) / ADC_MAX_VALUE) * refVoltage_;
        sumVoltage += voltage * ((r1_ + r2_) / r2_);
        delay(5);
    }
    currentVoltage_ = sumVoltage / static_cast<float>(WARMUP_SAMPLES);
#endif
}

float ADCBatteryMonitor::readVoltage() const {
    if (overrideActive_) return oVoltage_;
#ifndef NATIVE_BUILD
    int rawValue = analogRead(pin_);
    float voltage = (static_cast<float>(rawValue) / ADC_MAX_VALUE) * refVoltage_;
    float rawVoltage = voltage * ((r1_ + r2_) / r2_);
    currentVoltage_ = EMA_ALPHA * rawVoltage + (1.0f - EMA_ALPHA) * currentVoltage_;
#endif
    return currentVoltage_;
}

bool ADCBatteryMonitor::isLow() const {
    return readVoltage() < LOW_VOLTAGE_THRESHOLD;
}

void ADCBatteryMonitor::setOverride(float voltage) {
    oVoltage_ = voltage;
}
