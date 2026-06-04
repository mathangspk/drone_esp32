#include "core/KalmanFilter.h"

KalmanFilter::KalmanFilter(float initialState, float initialUncertainty)
    : state_(initialState), uncertainty_(initialUncertainty) {}

void KalmanFilter::update(float rate, float measurement, float dt) {
    // 1. Predict state (prior)
    state_ = state_ + dt * rate;

    // 2. Predict uncertainty (process noise standard deviation is 4 deg/s)
    uncertainty_ = uncertainty_ + dt * dt * 16.0f;

    // 3. Compute Kalman Gain (measurement noise standard deviation is 3 deg)
    float gain = uncertainty_ / (uncertainty_ + 9.0f);

    // 4. Update state with measurement
    state_ = state_ + gain * (measurement - state_);

    // 5. Update uncertainty (posterior)
    uncertainty_ = (1.0f - gain) * uncertainty_;
}

void KalmanFilter::reset(float state, float uncertainty) {
    state_ = state;
    uncertainty_ = uncertainty;
}
