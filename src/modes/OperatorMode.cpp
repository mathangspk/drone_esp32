#include "modes/OperatorMode.h"
#include <Arduino.h>
extern bool sendOperatorData; // Khai báo extern để dùng flag toàn cục

void OperatorMode::handle(
    float roll, float pitch, float yaw,
    float throttlePercent,
    PID& pidRoll, PID& pidPitch, PID& pidYaw,
    IBusReceiver& receiver,
    ESCController& escController
) {
    static float yaw_offset = 0;
    static bool lastArm = false;
    static bool everDisarmed = false;
    uint16_t rawThrottle = receiver.get_throttle();
    float newThrottle = (rawThrottle - 1000) * 100.0f / 1000.0f;
    if (newThrottle < 0) newThrottle = 0;
    if (newThrottle > 100) newThrottle = 100;
    throttlePercent = newThrottle; // Cập nhật giá trị mới nhất

    float rollSetpoint = map(receiver.get_roll(), 1000, 2000, -50, 50);
    float pitchSetpoint = map(receiver.get_pitch(), 1000, 2000, -50, 50);
    float yawSetpoint = map(receiver.get_yaw(), 1000, 2000, -50, 50);

    int aux1 = receiver.get_aux1();
    Serial.printf("Aux1: %d\n", aux1);
    if (aux1 <= 1500) {
        everDisarmed = true;
    }

    bool arm = (aux1 > 1500) && everDisarmed;    
   
    // Cập nhật offset khi vừa tắt arm
        if (!arm) {
            yaw_offset = yaw;
            escController.stopAll(); // Dừng động cơ nếu không ở trạng thái armed
            //Serial.println("Động cơ đã được tắt.");
        } else {   
            float yawInput = yaw;
            float yawProcessed = yaw - yaw_offset;
            Serial.printf("Yaw: %.2f, YawProcessed: %.2f, YawOffset: %.2f\n", yawInput, yawProcessed, yaw_offset);
            float dt = 1.0f / 200.0f;

            float roll_PID = pidRoll.compute(rollSetpoint, roll, dt);
            float pitch_PID = pidPitch.compute(pitchSetpoint, pitch, dt);
            float yaw_PID = pidYaw.compute(yawSetpoint, yawProcessed, dt);

            escController.mixAndSetMotors(throttlePercent, roll_PID, pitch_PID, yaw_PID);

            if (sendOperatorData) {
                float motor[4];
                motor[ESC_FL] = throttlePercent - pitch_PID + roll_PID - yaw_PID;
                motor[ESC_FR] = throttlePercent - pitch_PID - roll_PID + yaw_PID;
                motor[ESC_RL] = throttlePercent + pitch_PID + roll_PID + yaw_PID;
                motor[ESC_RR] = throttlePercent + pitch_PID - roll_PID - yaw_PID;
                // Lấy độ lớn tuyệt đối để điều khiển (không quan tâm chiều)
                for (int i = 0; i < 4; i++) {
                    motor[i] = fabs(motor[i]);
                    if (motor[i] > 100.0f) motor[i] = 100.0f;
            }
        Serial.printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
        millis(),
        throttlePercent,
        roll, pitch, yaw,
        roll_PID, pitch_PID, yaw_PID,
        rollSetpoint, pitchSetpoint, yawSetpoint,
        motor[ESC_FL], motor[ESC_FR], motor[ESC_RL], motor[ESC_RR]
        );
        }
        }
}