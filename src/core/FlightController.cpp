#include "core/FlightController.h"
#ifndef NATIVE_BUILD
#include "network/WebDashboardHandlers.h"
#endif

FlightController::FlightController(IIMU& imu, IPPM& ppm, IMotors& motors, IBattery& battery)
    : imu_(imu), ppm_(ppm), motors_(motors), battery_(battery) {}

void FlightController::init() {
    reset();
    loadPIDGains();
    calibrateGyro();
}

void FlightController::calibrateGyro() {
    float totalRoll = 0, totalPitch = 0, totalYaw = 0;
    const int kCalibrationSamples = 2000;
    for (int i = 0; i < kCalibrationSamples; ++i) {
        imu_.readSensor();
        float r, p, y;
        imu_.getGyroRates(r, p, y);
        totalRoll += r; totalPitch += p; totalYaw += y;
#ifndef NATIVE_BUILD
        delayMicroseconds(1000); // Wait for next independent IMU sample
#endif
    }
    calRollRate_ = totalRoll / static_cast<float>(kCalibrationSamples);
    calPitchRate_ = totalPitch / static_cast<float>(kCalibrationSamples);
    calYawRate_ = totalYaw / static_cast<float>(kCalibrationSamples);
}

void FlightController::reset() {
    rollRatePid_.reset(); pitchRatePid_.reset(); yawRatePid_.reset();
    rollAnglePid_.reset(); pitchAnglePid_.reset();
    motors_.writeMotors(1000, 1000, 1000, 1000);
}

void FlightController::update(float dt) {
    imu_.readSensor();
    ppm_.readChannels();

    if (ppm_.isSignalLost()) {
        reset();
        return;
    }

    bool isArmed = ppm_.getChannel(ARM_CHANNEL) > ARM_THRESHOLD;
    if (!isArmed) {
        if (wasArmed_) { reset(); wasArmed_ = false; }
        return;
    }
    if (!wasArmed_) {
        if (ppm_.getChannel(THROTTLE_CHANNEL) >= THROTTLE_IDLE_LIMIT) return;
        loadPIDGains();
        wasArmed_ = true;
    }

    float rateRoll, ratePitch, rateYaw, accRoll, accPitch;
    imu_.getGyroRates(rateRoll, ratePitch, rateYaw);
    rateRoll -= calRollRate_; ratePitch -= calPitchRate_; rateYaw -= calYawRate_;
    imu_.getAccAngles(accRoll, accPitch);

    rollKf_.update(rateRoll, accRoll, dt);
    pitchKf_.update(ratePitch, accPitch, dt);

    float desiredAngleRoll  = ROLL_SENSITIVITY  * (ppm_.getChannel(ROLL_CHANNEL)     - RC_CENTER);
    float desiredAnglePitch = PITCH_SENSITIVITY * (ppm_.getChannel(PITCH_CHANNEL)    - RC_CENTER);
    float inputThrottle     =  static_cast<float>(ppm_.getChannel(THROTTLE_CHANNEL));
    float desiredRateYaw    = YAW_SENSITIVITY   * (ppm_.getChannel(YAW_CHANNEL)       - RC_CENTER);

    float desiredRateRoll  = rollAnglePid_.update(desiredAngleRoll - rollKf_.getState(), rollKf_.getState(), dt);
    float desiredRatePitch = pitchAnglePid_.update(desiredAnglePitch - pitchKf_.getState(), pitchKf_.getState(), dt);

    float inputRoll  = rollRatePid_.update(desiredRateRoll - rateRoll, rateRoll, dt);
    float inputPitch = pitchRatePid_.update(desiredRatePitch - ratePitch, ratePitch, dt);
    float inputYaw   = yawRatePid_.update(desiredRateYaw - rateYaw, rateYaw, dt);

    if (inputThrottle > THROTTLE_MAX) inputThrottle = THROTTLE_MAX;
    int m[4] = {
        (int)(MIXING_SCALE * (inputThrottle - inputRoll - inputPitch - inputYaw)),
        (int)(MIXING_SCALE * (inputThrottle - inputRoll + inputPitch + inputYaw)),
        (int)(MIXING_SCALE * (inputThrottle + inputRoll + inputPitch - inputYaw)),
        (int)(MIXING_SCALE * (inputThrottle + inputRoll - inputPitch + inputYaw))};
    // Rescale all motors together to preserve attitude authority at saturation
    int hi = m[0], lo = m[0];
    for (int i = 1; i < 4; ++i) { if (m[i] > hi) hi = m[i]; if (m[i] < lo) lo = m[i]; }
    if (hi > MOTOR_MAX_US)       { int d = hi - MOTOR_MAX_US;       for (int i = 0; i < 4; ++i) m[i] -= d; }
    if (lo < MOTOR_MIN_ARMED_US) { int d = MOTOR_MIN_ARMED_US - lo; for (int i = 0; i < 4; ++i) m[i] += d; }
    for (int i = 0; i < 4; ++i) {
        if      (m[i] > MOTOR_MAX_US)       m[i] = MOTOR_MAX_US;
        else if (m[i] < MOTOR_MIN_ARMED_US) m[i] = MOTOR_MIN_ARMED_US;
    }

    if (ppm_.getChannel(THROTTLE_CHANNEL) < THROTTLE_IDLE_LIMIT) {
        m[0] = m[1] = m[2] = m[3] = 1000;
        reset();
    }

    motors_.writeMotors(m[0], m[1], m[2], m[3]);

#ifndef NATIVE_BUILD
    if (++logDiv_ >= 5) {
        WebDashboardHandlers::logFlightData(
            desiredAngleRoll,  rollKf_.getState(),
            desiredAnglePitch, pitchKf_.getState(),
            desiredRateYaw,    rateYaw,
            static_cast<int16_t>(inputThrottle),
            static_cast<int16_t>(m[0]), static_cast<int16_t>(m[1]),
            static_cast<int16_t>(m[2]), static_cast<int16_t>(m[3]),
            battery_.readVoltage());
        logDiv_ = 0;
    }
#endif
}
