#pragma once

class PID {
public:
    float kp, ki, kd;
    float integral, prevError;
    float outputMin, outputMax;

    PID(float kp, float ki, float kd, float outputMin = -50, float outputMax = 50);

    float compute(float setpoint, float measured, float dt);
    void reset();
};