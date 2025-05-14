#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>

class BatteryMonitor
{
public:
    BatteryMonitor(uint8_t adcPin, float vRef = 3.7466, int adcMax = 4095, float scale = 4.072483, float offset = 0);
    void begin();
    float readVoltage(); // Trả về điện áp thực tế (V)
    int readRaw();       // Trả về giá trị ADC thô

private:
    uint8_t _adcPin;
    float _vRef;
    int _adcMax;
    float _scale;
    float _offset;
    float interpolate(float x, float x0, float x1, float y0, float y1);
};

#endif
