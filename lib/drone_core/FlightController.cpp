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
    const float voltage = battery_.readVoltage();

    if (voltage < 9.0f || ppm_.isSignalLost()) {
        reset();
        return;
    }

    bool isArmed = ppm_.getChannel(4) > 1500;
    if (!isArmed) {
        if (wasArmed_) { reset(); wasArmed_ = false; }
        return;
    }
    if (!wasArmed_) { loadPIDGains(); wasArmed_ = true; }

    float rateRoll, ratePitch, rateYaw, accRoll, accPitch;
    imu_.getGyroRates(rateRoll, ratePitch, rateYaw);
    rateRoll -= calRollRate_; ratePitch -= calPitchRate_; rateYaw -= calYawRate_;
    imu_.getAccAngles(accRoll, accPitch);

    rollKf_.update(rateRoll, accRoll, dt);
    pitchKf_.update(ratePitch, accPitch, dt);

    float desiredAngleRoll  = 0.10f * (ppm_.getChannel(0) - 1500);
    float desiredAnglePitch = 0.10f * (ppm_.getChannel(1) - 1500);
    float inputThrottle     = ppm_.getChannel(2);
    float desiredRateYaw    = 0.15f * (ppm_.getChannel(3) - 1500);

    float desiredRateRoll  = rollAnglePid_.update(desiredAngleRoll - rollKf_.getState(), rollKf_.getState(), dt);
    float desiredRatePitch = pitchAnglePid_.update(desiredAnglePitch - pitchKf_.getState(), pitchKf_.getState(), dt);

    float inputRoll  = rollRatePid_.update(desiredRateRoll - rateRoll, rateRoll, dt);
    float inputPitch = pitchRatePid_.update(desiredRatePitch - ratePitch, ratePitch, dt);
    float inputYaw   = yawRatePid_.update(desiredRateYaw - rateYaw, rateYaw, dt);

    if (inputThrottle > 1800.0f) inputThrottle = 1800.0f;
    int m[4] = {
        (int)(1.024f * (inputThrottle - inputRoll - inputPitch - inputYaw)),
        (int)(1.024f * (inputThrottle - inputRoll + inputPitch + inputYaw)),
        (int)(1.024f * (inputThrottle + inputRoll + inputPitch - inputYaw)),
        (int)(1.024f * (inputThrottle + inputRoll - inputPitch + inputYaw))};
    // Rescale all motors together to preserve attitude authority at saturation
    int hi = m[0], lo = m[0];
    for (int i = 1; i < 4; ++i) { if (m[i] > hi) hi = m[i]; if (m[i] < lo) lo = m[i]; }
    if (hi > 2000) { int d = hi - 2000; for (int i = 0; i < 4; ++i) m[i] -= d; }
    if (lo < 1180) { int d = 1180 - lo; for (int i = 0; i < 4; ++i) m[i] += d; }
    for (int i = 0; i < 4; ++i) { if (m[i] > 2000) m[i] = 2000; else if (m[i] < 1180) m[i] = 1180; }

    if (ppm_.getChannel(2) < 1050) {
        m[0] = m[1] = m[2] = m[3] = 1000;
        reset();
    }

    motors_.writeMotors(m[0], m[1], m[2], m[3]);

#ifndef NATIVE_BUILD
    if (++logDiv_ >= 5) {
        WebDashboardHandlers::logFlightData(desiredAngleRoll, rollKf_.getState(),
                                           desiredAnglePitch, pitchKf_.getState(),
                                           static_cast<int16_t>(inputThrottle));
        logDiv_ = 0;
    }
#endif
}
