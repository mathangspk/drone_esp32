#pragma once

class PIDController {
public:
    PIDController(float kp, float ki, float kd, float outMin = -1.0f, float outMax = 1.0f);

    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void reset();

    // target: giá trị mong muốn, input: giá trị đo được, dt: thời gian chu kỳ (giây)
    float compute(float target, float input, float dt);

private:
    float _kp, _ki, _kd;
    float _outMin, _outMax;
    float _integral;
    float _prevError;
};