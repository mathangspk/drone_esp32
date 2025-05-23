#include "currentMeasure/CurrentMeasure.h"

CurrentMonitor::CurrentMonitor(uint8_t adcPin, float vRef, int adcMax, float scale, float offset)
    : _adcPin(adcPin), _vRef(vRef), _adcMax(adcMax), _scale(scale), _offset(offset) {}

void CurrentMonitor::begin()
{
    analogReadResolution(12);                   // ESP32 mặc định 12-bit
    analogSetPinAttenuation(_adcPin, ADC_11db); // cho phép đo đến ~3.3V
}

int CurrentMonitor::readRaw()
{
    return analogRead(_adcPin);
}

// Hàm nội suy tuyến tính giữa hai điểm
float CurrentMonitor::interpolate(float x, float x0, float x1, float y0, float y1) {
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

float CurrentMonitor::readVoltage()
{
    int raw = readRaw();
    //float adcVoltage = raw * 0.000722 + 0.210;
    //float voltage = adcVoltage * _scale + _offset;
    float adcVoltage;
    //Need to be measure and fill the value
     if (raw <= 1535) {
        adcVoltage = interpolate(raw, 1195, 1535, 1.120, 1.351); //5.5
     }
    else if (raw <= 1700) {
    adcVoltage = interpolate(raw, 1535, 1700, 1.351, 1.478); //6.0
    } else if (raw <= 2005) {
    adcVoltage = interpolate(raw, 1700, 2005, 1.478, 1.723); //7.0
    } else if (raw <= 2318) {
    adcVoltage =  interpolate(raw, 2005, 2318, 1.723, 1.971); //8.0
    } else if (raw <= 2622) {
    adcVoltage =  interpolate(raw, 2318, 2622, 1.971, 2.212); //9.0
    } else if (raw <= 2944) {
    adcVoltage =  interpolate(raw, 2622, 2944, 2.212, 2.463); //10.0
    } else if (raw <= 3297) {
    adcVoltage =  interpolate(raw, 2944, 3297, 2.463, 2.706); //11.0
    } else if (raw <= 3729) {
    adcVoltage =  interpolate(raw, 3297, 3729, 2.706, 2.938); //12.0
     } else if (raw <= 4304) {
    adcVoltage =  interpolate(raw, 3729, 4304, 2.938, 3.057); //12.4
    } else {
        adcVoltage =  interpolate(raw, 4304, 4058, 3.057, 3.197); //13.0
    }
    float current = 10*(adcVoltage * _scale - 2.5);
    if (current < 0) {
        current = 0;
    }
    //Serial.printf("ADC Raw: %d, Pin Voltage: %.3fV, Current: %.3fA\n", raw, adcVoltage, current);
    return current;
}


