#include "hardware/ADCBatteryMonitor.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

ADCBatteryMonitor::ADCBatteryMonitor(int analogPin, float refVoltage, float r1Value, float r2Value)
    : pin_(analogPin), refVoltage_(refVoltage), r1_(r1Value), r2_(r2Value) {}

void ADCBatteryMonitor::init() {
#ifndef NATIVE_BUILD
    analogReadResolution(12);
    pinMode(pin_, INPUT);
    
    // Warm up the filter to establish a correct baseline immediately on boot
    float sumVoltage = 0.0f;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; ++i) {
        int rawValue = analogRead(pin_);
        float voltage = (static_cast<float>(rawValue) / 4095.0f) * refVoltage_;
        sumVoltage += voltage * ((r1_ + r2_) / r2_);
        delay(5);
    }
    currentVoltage_ = sumVoltage / static_cast<float>(numSamples);
#endif
}

float ADCBatteryMonitor::readVoltage() const {
    if (overrideActive_) return oVoltage_;
#ifndef NATIVE_BUILD
    int rawValue = analogRead(pin_);
    float voltage = (static_cast<float>(rawValue) / 4095.0f) * refVoltage_;
    float rawVoltage = voltage * ((r1_ + r2_) / r2_);
    
    // Exponential Moving Average filter (alpha = 0.05) to eliminate high frequency ADC noise
    currentVoltage_ = 0.05f * rawVoltage + 0.95f * currentVoltage_;
#endif
    return currentVoltage_;
}

bool ADCBatteryMonitor::isLow() const {
    return readVoltage() < 9.0f;
}

void ADCBatteryMonitor::setOverride(float voltage) {
    oVoltage_ = voltage;
}
