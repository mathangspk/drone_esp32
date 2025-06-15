#pragma once
#include <ESCcontrollers/PIDESC.h>
#include <receiver/IBusReceiver.h>
#include <ESCcontrollers/ESCControllers.h>

class OperatorMode {
public:
    void handle(
        float roll, float pitch, float yaw,
        float throttlePercent,
        PID& pidRoll, PID& pidPitch, PID& pidYaw,
        IBusReceiver& receiver,
        ESCController& escController
    );
};