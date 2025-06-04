#include "inputs/AnalogInput.h"

AnalogInput::AnalogInput(uint8_t pin, float scale, float offset, float multiplier, float alpha)
  : _pin(pin), _scale(scale), _offset(offset), _multiplier(multiplier), _alpha(alpha), _filteredValue(0.0f) {}

void AnalogInput::begin() {
    pinMode(_pin, INPUT);
    analogReadResolution(12);                   // ESP32 mặc định 12-bit
    analogSetPinAttenuation(_pin, ADC_11db); // cho phép đo đến ~3.3V
    for (int i = 0; i < NUM_SAMPLES; i++) {
        _samples[i] = 0;
    }

}
// Hàm nội suy tuyến tính giữa hai điểm
float AnalogInput::interpolate(float x, float x0, float x1, float y0, float y1) {
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

void AnalogInput::update() {
    float raw = analogRead(_pin);  // raw từ 0 đến 4095
    raw = computeAverage(raw);
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
    float voltageScaled = adcVoltage * _scale + _offset;
    //Serial.printf("ADC Raw: %d, Pin Voltage: %.3fV, Scaled Voltage: %.3f\n", raw, adcVoltage, voltageScaled);
    _filteredValue = _alpha * voltageScaled + (1.0 - _alpha) * _filteredValue;
}

float AnalogInput::getRaw() {
    return analogRead(_pin);  // Chỉ lấy 1 lần (nếu cần)
}

float AnalogInput::getFiltered() {
    return _filteredValue * _multiplier;
}

float AnalogInput::computeAverage(float newValue) {
    _samples[_sampleIndex] = newValue;
    _sampleIndex = (_sampleIndex + 1) % NUM_SAMPLES;

    if (_sampleIndex == 0) _bufferFilled = true;

    int count = _bufferFilled ? NUM_SAMPLES : _sampleIndex;

    float sum = 0;
    for (int i = 0; i < count; i++) {
        sum += _samples[i];
    }

    return sum / count;
}
