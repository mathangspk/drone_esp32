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

// Hàm nội suy tuyến tính giữa hai điểm
float BatteryMonitor::interpolate(float x, float x0, float x1, float y0, float y1) {
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

float BatteryMonitor::readVoltage()
{
    int raw = readRaw();
    //float adcVoltage = raw * 0.000722 + 0.210;
    //float voltage = adcVoltage * _scale + _offset;
    float adcVoltage;
    //Need to be measure and fill the value
     if (raw <= 1537) {
        adcVoltage = interpolate(raw, 1195, 1537, 1.088, 1.40);
    } else if (raw <= 2804) {
        adcVoltage = interpolate(raw, 1537, 2804, 1.40, 2.15);
    } else if (raw <= 3717) {
        adcVoltage =  interpolate(raw, 2804, 3717, 2.15, 2.85);
    } else {
        adcVoltage =  interpolate(raw, 3717, 4304, 2.85, 3.30);
    }
    float voltage = adcVoltage * _scale + _offset;
    //Serial.printf("ADC Raw: %d, Pin Voltage: %.3fV, Battery Voltage: %.3fV\n", raw, adcVoltage, voltage);
    return voltage;
}


