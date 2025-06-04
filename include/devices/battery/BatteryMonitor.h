#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "inputs/AnalogInput.h"

class BatteryMonitor {
public:
    BatteryMonitor(AnalogInput* analogInput);  // truyền vào đối tượng AnalogInput đã khởi tạo
    void update();                              // cập nhật giá trị
    float getVoltage();                         // trả về điện áp đã lọc
    float getRawADC();                          // trả về giá trị ADC thô

private:
    AnalogInput* _analogInput;
    float _voltage;
};

#endif
