#ifndef ANALOG_INPUT_H
#define ANALOG_INPUT_H

#include <Arduino.h>

#define NUM_SAMPLES 20  // Số mẫu dùng để lọc

class AnalogInput {
public:
    AnalogInput(uint8_t pin, float scale = 0, float offset = 0, float multiplier = 1.0, float alpha = 0);

    void begin();               // Thiết lập nếu cần
    void update();              // Gọi định kỳ để cập nhật giá trị
    float getRaw();             // Trả về giá trị chưa nhân
    float getFiltered();        // Trả về giá trị đã lọc + nhân hệ số

private:
    uint8_t _pin;
    float _multiplier;
    float _alpha;
    float _filteredValue;
    float _scale;
    float _offset;
    float interpolate(float x, float x0, float x1, float y0, float y1);
    float _samples[NUM_SAMPLES];
    int _sampleIndex = 0;
    bool _bufferFilled = false;

    float computeAverage(float newValue);  // Hàm lọc trung bình
    float _filteredVoltage = 0;
};

#endif
