#include "orientation/MadgwickAHRS.h"
#include <math.h>

MadgwickAHRS::MadgwickAHRS(float beta) : beta(beta), q0(1.0f), q1(0), q2(0), q3(0) {}

void MadgwickAHRS::update(float gx, float gy, float gz, float ax, float ay, float az, float dt) {
    float norm = sqrt(ax * ax + ay * ay + az * az);
    if (norm == 0.0f) return;
    ax /= norm; ay /= norm; az /= norm;

    float _2q0 = 2.0f * q0;
    float _2q1 = 2.0f * q1;
    float _2q2 = 2.0f * q2;
    float _2q3 = 2.0f * q3;
    float _4q0 = 4.0f * q0;
    float _4q1 = 4.0f * q1;
    float _4q2 = 4.0f * q2;
    float _8q1 = 8.0f * q1;
    float _8q2 = 8.0f * q2;
    float q0q0 = q0 * q0;
    float q1q1 = q1 * q1;
    float q2q2 = q2 * q2;
    float q3q3 = q3 * q3;

    float s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
    float s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
    float s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
    float s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
    norm = sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    s0 /= norm; s1 /= norm; s2 /= norm; s3 /= norm;

    float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - beta * s0;
    float qDot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy) - beta * s1;
    float qDot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx) - beta * s2;
    float qDot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx) - beta * s3;

    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;

    norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 /= norm; q1 /= norm; q2 /= norm; q3 /= norm;
}

float MadgwickAHRS::getRoll() const {
    return atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * 180.0f / M_PI;
}

float MadgwickAHRS::getPitch() const {
    float sinp = 2.0f * (q0 * q2 - q3 * q1);
    if (fabs(sinp) >= 1)
        return copysignf(90.0f, sinp);
    else
        return asinf(sinp) * 180.0f / M_PI;
}

float MadgwickAHRS::getYaw() const {
    return atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * 180.0f / M_PI;
}
