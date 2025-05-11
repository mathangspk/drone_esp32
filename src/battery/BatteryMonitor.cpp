#include "battery/BatteryMonitor.h"

BatteryMonitor::BatteryMonitor(uint8_t adcPin, float vRef, int adcMax, float scale, float offset)
    : _adcPin(adcPin), _vRef(vRef), _adcMax(adcMax), _scale(scale), _offset(offset) {}

void BatteryMonitor::begin()
{
    analogReadResolution(12);                   // ESP32 mặc định 12-bit
    analogSetPinAttenuation(_adcPin, ADC_11db); // cho phép đo đến ~3.3V
}

int BatteryMonitor::readRaw()
{
    return analogRead(_adcPin);
}

float BatteryMonitor::readVoltage()
{
    int raw = readRaw();
    float adcVoltage = raw * 0.000722 + 0.210;
    float voltage = adcVoltage * _scale + _offset;

    //Serial.printf("ADC Raw: %d, Pin Voltage: %.3fV, Battery Voltage: %.3fV\n", raw, adcVoltage, voltage);
    return voltage;
}
