#include "modes/ConfigMode.h"
#include <Arduino.h>

void ConfigMode::handle(
    const String& option, // Thêm dòng này
    float roll, float pitch, float yaw,
    float ax, float ay, float az,
    float gx, float gy, float gz,
    float mx, float my, float mz,
    IBusReceiver& receiver
) {
    if (option == "orientation") {
        Serial.printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            millis(),
            roll, pitch, yaw,
            ax, ay, az,
            gx, gy, gz,
            mx, my, mz
        );
    } else if (option == "ia6b") {
        Serial.printf("%lu,%u,%u,%u,%u,%u,%u\n",
            millis(),
            receiver.get_throttle(),
            receiver.get_roll(),
            receiver.get_pitch(),
            receiver.get_yaw(),
            receiver.get_aux1(),
            receiver.get_aux2()
        );
    }
}