#pragma once
#include <receiver/IBusReceiver.h>
class ConfigMode {
public:
    void handle(
        const String& option,
        float roll, float pitch, float yaw,
        float ax, float ay, float az,
        float gx, float gy, float gz,
        float mx, float my, float mz,
        IBusReceiver& receiver
    );
};