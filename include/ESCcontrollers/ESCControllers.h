#ifndef ESC_CONTROLLER_H
#define ESC_CONTROLLER_H

#include <Arduino.h>

enum ESC_ID
{
    ESC_FL = 0,
    ESC_FR,
    ESC_RL,
    ESC_RR
};

class ESCController
{
public:
    ESCController();
    void begin();
    void setPulseRange(ESC_ID escId, uint16_t minP, uint16_t maxP);
    void setESCValue(ESC_ID escId, uint8_t percent); // percent: 0-100
    void stopAll();
    uint8_t getCurrentValue(ESC_ID esc);

    // Hàm mix động cơ cho quad X
    // throttle: 0-100 (%), roll/pitch/yaw: -1.0 ~ 1.0
    void mixAndSetMotors(float throttle, float roll, float pitch, float yaw);
private:
    static const int escPins[4];
    static const int escChannels[4];
    uint16_t minPulse[4];
    uint16_t maxPulse[4];
    uint8_t escValues[4];

    uint32_t microsecondsToDuty(uint16_t us);
};

#endif
