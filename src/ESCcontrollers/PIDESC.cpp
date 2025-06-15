#include "ESCControllers/PIDESC.h"

PID::PID(float kp, float ki, float kd, float outputMin, float outputMax)
    : kp(kp), ki(ki), kd(kd), integral(0), prevError(0), outputMin(outputMin), outputMax(outputMax) {}

float PID::compute(float setpoint, float measured, float dt) {
    float error = setpoint - measured;
    integral += error * dt;
    float derivative = (error - prevError) / dt;
    float output = kp * error + ki * integral + kd * derivative;
    prevError = error;
    if (output > outputMax) output = outputMax;
    if (output < outputMin) output = outputMin;
    return output;
}

void PID::reset() {
    integral = 0;
    prevError = 0;
}