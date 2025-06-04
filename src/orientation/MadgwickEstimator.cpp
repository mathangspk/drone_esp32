#include "orientation/MadgwickEstimator.h"
#include <math.h>

MadgwickEstimator::MadgwickEstimator()
    : q0(1), q1(0), q2(0), q3(0), roll(0), pitch(0), yaw(0), yawOffset(0), anglesComputed(false), yawGyro(0) {}

void MadgwickEstimator::begin() {
    q0 = 1.0f; q1 = q2 = q3 = 0.0f;
    roll = pitch = yaw = 0.0f;
    yawGyro = 0.0f;
    yawOffset = 0.0f;
    anglesComputed = false;
}

void MadgwickEstimator::update(float ax, float ay, float az,
                               float gx, float gy, float gz, float dt) {
    // Nếu cần, chuyển đổi deg/s sang rad/s ở đây
    // gx *= DEG_TO_RAD; gy *= DEG_TO_RAD; gz *= DEG_TO_RAD;

    // Madgwick filter cho roll, pitch (giữ nguyên)
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot0, qDot1, qDot2, qDot3;

    // Tính đạo hàm quaternion từ gyro
    qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    qDot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    qDot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    qDot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    // Bình thường hóa gia tốc kế
    recipNorm = sqrtf(ax * ax + ay * ay + az * az);
    if (recipNorm == 0.0f) return;
    recipNorm = 1.0f / recipNorm;
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;

    // Gradient descent algorithm corrective step
    s0 = 4.0f * q0 * (q1 * q1 + q2 * q2) + 2.0f * q2 * ax + 2.0f * q1 * ay;
    s1 = 4.0f * q1 * q3 * q3 - 2.0f * q3 * ax + 4.0f * q0 * q0 * q1 - 2.0f * q0 * az;
    s2 = 4.0f * q2 * q3 * q3 + 2.0f * q3 * ay + 4.0f * q0 * q0 * q2 - 2.0f * q0 * ax;
    s3 = 4.0f * q1 * q1 * q3 + 4.0f * q2 * q2 * q3;

    recipNorm = sqrtf(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    recipNorm = 1.0f / recipNorm;

    s0 *= recipNorm;
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;

    // Áp dụng phản hồi
    qDot0 -= 0.1f * s0;
    qDot1 -= 0.1f * s1;
    qDot2 -= 0.1f * s2;
    qDot3 -= 0.1f * s3;

    // Tích phân để cập nhật quaternion
    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;

    // Chuẩn hóa quaternion
    recipNorm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    recipNorm = 1.0f / recipNorm;
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    // Tích phân gyroZ để tính yawGyro (đơn vị: radian)
    yawGyro += gz * dt;

    anglesComputed = false;  // Bắt buộc tính lại Euler angles
}

void MadgwickEstimator::computeAngles() {
    roll = atan2f(2.0f * (q0 * q1 + q2 * q3),
                  1.0f - 2.0f * (q1 * q1 + q2 * q2));
    pitch = asinf(2.0f * (q0 * q2 - q3 * q1));
    // Không dùng yaw từ quaternion nữa!
    anglesComputed = true;
}

float MadgwickEstimator::getRoll() const {
    if (!anglesComputed) const_cast<MadgwickEstimator*>(this)->computeAngles();
    return roll * 180.0f / M_PI;
}

float MadgwickEstimator::getPitch() const {
    if (!anglesComputed) const_cast<MadgwickEstimator*>(this)->computeAngles();
    return pitch * 180.0f / M_PI;
}

float MadgwickEstimator::getYaw() const {
    // Trả về yaw tính bằng gyroZ tích phân, đã trừ offset, đơn vị độ
    return (yawGyro - yawOffset) * 180.0f / M_PI;
}

void MadgwickEstimator::calibrateYaw() {
    yawOffset = yawGyro;
}