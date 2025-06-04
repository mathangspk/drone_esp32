#include "devices/currentMeasure/CurrentMeasure.h"

CurrentMonitor::CurrentMonitor(AnalogInput* analogInput)
  : _analogInput(analogInput), currentProcess(0.0f), _current(0.0f) {}

void CurrentMonitor::update() {
    _analogInput->update();
    currentProcess = (_analogInput->getFiltered()-2.54)/0.1; // hoặc áp dụng thêm công thức tính dòng nếu cần
    _current = _alpha * currentProcess + (1.0 - _alpha) * _current;
}

float CurrentMonitor::getCurrent() {
    //Serial.printf(" Current: %.3fA\n", _current);
    return _current;
}

float CurrentMonitor::getRawADC() {
    return _analogInput->getRaw();
}
