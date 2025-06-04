#include "devices/battery/BatteryMonitor.h"

BatteryMonitor::BatteryMonitor(AnalogInput* analogInput)
  : _analogInput(analogInput), _voltage(0.0f) {}

void BatteryMonitor::update() {
    _analogInput->update();
    _voltage = _analogInput->getFiltered();
}

float BatteryMonitor::getVoltage() {
    return _voltage;
}

float BatteryMonitor::getRawADC() {
    return _analogInput->getRaw();
}
