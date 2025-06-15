#include "ESCcontrollers/ESCControllers.h"

// Cấu hình chân điều khiển ESC và kênh PWM
const int ESCController::escPins[4] = {14, 27, 4, 26};
const int ESCController::escChannels[4] = {0, 1, 2, 3};

ESCController::ESCController()
{
    for (int i = 0; i < 4; ++i)
    {
        escValues[i] = 0;
        minPulse[i] = 1000;
        maxPulse[i] = 1700;
    }
}

void ESCController::begin()
{
    for (int i = 0; i < 4; ++i)
    {
        ledcSetup(escChannels[i], 50, 16); // 50Hz, 16-bit resolution
        ledcAttachPin(escPins[i], escChannels[i]);

        // Gửi xung trung lập 1000us để khởi tạo ESC
        ledcWrite(escChannels[i], microsecondsToDuty(1000));
    }
}
void ESCController::setPulseRange(ESC_ID escId, uint16_t minP, uint16_t maxP)
{
    minPulse[escId] = minP;
    maxPulse[escId] = maxP;
}
void ESCController::setESCValue(ESC_ID escId, uint8_t percent)
{
    percent = constrain(percent, 0, 100);
    escValues[escId] = percent;

    // PWM từ 1000µs (0%) đến 1700µs (100%)
    uint16_t pulse = map(percent, 0, 100, minPulse[escId], maxPulse[escId]);
    ledcWrite(escChannels[escId], microsecondsToDuty(pulse));
}

void ESCController::stopAll()
{
    for (int i = 0; i < 4; ++i)
    {
        escValues[i] = 0;
        ledcWrite(escChannels[i], microsecondsToDuty(1000)); // dừng = 1000us
    }
}

uint8_t ESCController::getCurrentValue(ESC_ID esc)
{
    return escValues[esc];
}

uint32_t ESCController::microsecondsToDuty(uint16_t us)
{
    return (uint32_t)us * 65535 / 20000; // 20ms = 1 chu kỳ ở 50Hz
}

void ESCController::mixAndSetMotors(float throttle, float roll, float pitch, float yaw)
{
    // Công thức mix chuẩn Quad X (giống Betaflight)
    // Motor order: FL, FR, RL, RR
    float motor[4];
    motor[ESC_FL] = throttle - pitch + roll - yaw; // Front Left (CCW)
    motor[ESC_FR] = throttle - pitch - roll + yaw; // Front Right (CW)
    motor[ESC_RL] = throttle + pitch + roll + yaw; // Rear Left (CW)
    motor[ESC_RR] = throttle + pitch - roll - yaw; // Rear Right (CCW)

    // Lấy độ lớn tuyệt đối để điều khiển (không quan tâm chiều)
    for (int i = 0; i < 4; i++) {
        motor[i] = fabs(motor[i]);
        if (motor[i] > 100.0f) motor[i] = 100.0f;
        setESCValue((ESC_ID)i, (uint8_t)motor[i]);
    }
}