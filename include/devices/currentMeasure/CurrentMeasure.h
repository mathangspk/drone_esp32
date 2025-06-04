#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include <Arduino.h>
#include "inputs/AnalogInput.h"

class CurrentMonitor {
public:
    CurrentMonitor(AnalogInput* analogInput);  // truyền vào đối tượng AnalogInput đã khởi tạo
    void update();                             // cập nhật giá trị
    float getCurrent();                        // trả về dòng điện tính toán
    float getRawADC();                         // trả về ADC thô

private:
    AnalogInput* _analogInput;
    float _current;
    float currentProcess;
    float _alpha = 0.1;
};

#endif
