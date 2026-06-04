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

    // Outer Angle PIDs
    PIDController rollAnglePid_{1.5f, 0.0f, 0.6f, 0.5f};
    PIDController pitchAnglePid_{1.5f, 0.0f, 0.6f, 0.5f};

    // Calibration Offsets
    float calRollRate_ = 0.0f;
    float calPitchRate_ = 0.0f;
    float calYawRate_ = 0.0f;

    bool wasArmed_ = false;
    int logDiv_ = 0;
};

#endif // FLIGHTCONTROLLER_H
