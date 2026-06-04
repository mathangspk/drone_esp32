#include "core/PIDController.h"

PIDController::PIDController(float kp, float ki, float kd, float dAlpha)
    : kp_(kp), ki_(ki), kd_(kd), dAlpha_(dAlpha) {}

float PIDController::update(float error, float measurement, float dt) {
    float pTerm = kp_ * error;

    float newIterm = iterm_ + ki_ * (error + prevError_) * dt / 2.0f;
    if (newIterm > 400.0f) newIterm = 400.0f;
    else if (newIterm < -400.0f) newIterm = -400.0f;
    iterm_ = newIterm;

    // D-on-measurement: negate to suppress derivative kick when setpoint changes
    float dRaw = (dt > 0.0f) ? -kd_ * (measurement - prevMeasurement_) / dt : 0.0f;
    dFiltered_ = dAlpha_ * dRaw + (1.0f - dAlpha_) * dFiltered_;

    float output = pTerm + iterm_ + dFiltered_;
    if (output > 400.0f) output = 400.0f;
    else if (output < -400.0f) output = -400.0f;

    prevError_ = error;
    prevMeasurement_ = measurement;
    return output;
}

float PIDController::update(float error, float prevError, float prevIterm, float dt) {
    float pTerm = kp_ * error;

    float newIterm = prevIterm + ki_ * (error + prevError) * dt / 2.0f;
    if (newIterm > 400.0f) newIterm = 400.0f;
    else if (newIterm < -400.0f) newIterm = -400.0f;
    iterm_ = newIterm;

    float dTerm = (dt > 0.0f) ? kd_ * (error - prevError) / dt : 0.0f;

    float output = pTerm + iterm_ + dTerm;
    if (output > 400.0f) output = 400.0f;
    else if (output < -400.0f) output = -400.0f;

    prevError_ = error;
    return output;
}

void PIDController::reset() {
    prevError_ = 0.0f;
    prevMeasurement_ = 0.0f;
    iterm_ = 0.0f;
    dFiltered_ = 0.0f;
}

void PIDController::setGains(float kp, float ki, float kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}
