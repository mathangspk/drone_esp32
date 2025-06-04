#include "PIDcontroller/PIDController.h"

PIDController::PIDController(float kp, float ki, float kd, float outMin, float outMax)
    : _kp(kp), _ki(ki), _kd(kd), _outMin(outMin), _outMax(outMax), _integral(0), _prevError(0) {}

void PIDController::setTunings(float kp, float ki, float kd) {
    _kp = kp; _ki = ki; _kd = kd;
}

void PIDController::setOutputLimits(float min, float max) {
    _outMin = min; _outMax = max;
}

void PIDController::reset() {
    _integral = 0;
    _prevError = 0;
}

float PIDController::compute(float target, float input, float dt) {
    float error = target - input;
    _integral += error * dt;
    float derivative = (dt > 0) ? (error - _prevError) / dt : 0;
    float output = _kp * error + _ki * _integral + _kd * derivative;
    if (output > _outMax) output = _outMax;
    if (output < _outMin) output = _outMin;
    _prevError = error;
    return output;
}