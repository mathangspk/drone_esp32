#ifndef FLIGHTCONTROLLER_H
#define FLIGHTCONTROLLER_H

#include "interfaces/IIMU.h"
#include "interfaces/IPPM.h"
#include "interfaces/IMotors.h"
#include "interfaces/IBattery.h"
#include "core/PIDController.h"
#include "core/KalmanFilter.h"

class FlightController {
public:
    FlightController(IIMU& imu, IPPM& ppm, IMotors& motors, IBattery& battery);

    void init();
    void update(float dt);
    void reset();

    // Calibration helper
    void calibrateGyro();

    // Load dynamic PID values from NVS memory
    void loadPIDGains();

private:
    // RC channel indices
    static constexpr int ROLL_CHANNEL     = 0;
    static constexpr int PITCH_CHANNEL    = 1;
    static constexpr int THROTTLE_CHANNEL = 2;
    static constexpr int YAW_CHANNEL      = 3;
    static constexpr int ARM_CHANNEL      = 4;

    // RC thresholds and stick scaling
    static constexpr int   RC_CENTER          = 1500; // center stick µs
    static constexpr int   ARM_THRESHOLD      = 1500; // AUX1 above this = armed
    static constexpr int   THROTTLE_IDLE_LIMIT = 1050; // below = idle, above = flying
    static constexpr float THROTTLE_MAX       = 1800.0f; // cap before motor mixing
    static constexpr float ROLL_SENSITIVITY   = 0.10f; // deg per µs from center
    static constexpr float PITCH_SENSITIVITY  = 0.10f;
    static constexpr float YAW_SENSITIVITY    = 0.15f; // deg/s per µs from center

    // Motor output limits and mixing
    static constexpr int   MOTOR_MAX_US       = 2000;
    static constexpr int   MOTOR_MIN_ARMED_US = 1180; // keeps ESCs spinning while armed
    static constexpr float MIXING_SCALE       = 1.024f;

    IIMU& imu_;
    IPPM& ppm_;
    IMotors& motors_;
    IBattery& battery_;

    // Filters
    KalmanFilter rollKf_;
    KalmanFilter pitchKf_;

    // Inner Rate PIDs — dAlpha=0.5 ≈ 40Hz LPF on D-term at 250Hz loop rate
    PIDController rollRatePid_{0.7f, 0.0f, 0.01f, 0.5f};
    PIDController pitchRatePid_{0.7f, 0.0f, 0.01f, 0.5f};
    PIDController yawRatePid_{2.0f, 12.0f, 0.0f};

    // Outer Angle PIDs — D-term starts at 0 to avoid noise amplification on first flights
    PIDController rollAnglePid_{1.5f, 0.0f, 0.0f, 0.5f};
    PIDController pitchAnglePid_{1.5f, 0.0f, 0.0f, 0.5f};

    // Calibration Offsets
    float calRollRate_ = 0.0f;
    float calPitchRate_ = 0.0f;
    float calYawRate_ = 0.0f;

    bool wasArmed_ = false;
    int logDiv_ = 0;
};

#endif // FLIGHTCONTROLLER_H
