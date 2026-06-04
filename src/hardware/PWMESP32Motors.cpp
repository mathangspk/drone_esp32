#include "hardware/PWMESP32Motors.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

PWMESP32Motors::PWMESP32Motors(int pinM1, int pinM2, int pinM3, int pinM4) {
    pins_[0] = pinM1; pins_[1] = pinM2; pins_[2] = pinM3; pins_[3] = pinM4;
}

void PWMESP32Motors::init() {
#ifndef NATIVE_BUILD
    for (int i = 0; i < 4; ++i) {
        pinMode(pins_[i], OUTPUT);
        // channels 0, 1, 2, 3, 250Hz frequency, 12-bit resolution
        ledcSetup(i, 250, 12);
        ledcAttachPin(pins_[i], i);
    }
#endif
}

void PWMESP32Motors::writeMotors(int m1, int m2, int m3, int m4) {
    outputs_[0] = m1; outputs_[1] = m2; outputs_[2] = m3; outputs_[3] = m4;
    
#ifndef NATIVE_BUILD
    for (int i = 0; i < 4; ++i) {
        int speed = oActive_[i] ? oVal_[i] : outputs_[i];
        ledcWrite(i, speed);
    }
#endif
}

void PWMESP32Motors::setOverride(int motorIdx, int value, bool active) {
    if (motorIdx >= 0 && motorIdx < 4) {
        oActive_[motorIdx] = active;
        oVal_[motorIdx] = value;
    }
}

int PWMESP32Motors::getMotorOutput(int motorIdx) const {
    if (motorIdx >= 0 && motorIdx < 4) {
        return oActive_[motorIdx] ? oVal_[motorIdx] : outputs_[motorIdx];
    }
    return 1000;
}

bool PWMESP32Motors::isMotorOverridden(int motorIdx) const {
    return (motorIdx >= 0 && motorIdx < 4) ? oActive_[motorIdx] : false;
}
