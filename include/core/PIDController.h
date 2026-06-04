#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

/**
 * @brief Cascaded PID controller with D-on-measurement and optional D-term LPF.
 * dAlpha = 1.0 means no filtering; lower values cut high-frequency noise.
 */
class PIDController {
public:
    PIDController(float kp, float ki, float kd, float dAlpha = 1.0f);

    // Main update: D acts on measurement (no setpoint kick), LPF applied.
    float update(float error, float measurement, float dt);

    // Test utility: explicit state injection with D-on-error (no LPF).
    float update(float error, float prevError, float prevIterm, float dt);

    void reset();
    void setGains(float kp, float ki, float kd);

    float getIterm() const { return iterm_; }
    float getError() const { return prevError_; }

private:
    static constexpr float kOutputLimit = 400.0f; // motor mixing range ±400µs

    float kp_, ki_, kd_, dAlpha_;
    float prevError_ = 0.0f;
    float prevMeasurement_ = 0.0f;
    float iterm_ = 0.0f;
    float dFiltered_ = 0.0f;
};

#endif // PIDCONTROLLER_H
