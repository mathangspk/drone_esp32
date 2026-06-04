#ifndef MPU6050IMU_H
#define MPU6050IMU_H

#include "interfaces/IIMU.h"

/**
 * @brief ESP32 hardware driver for the MPU6050 accelerometer and gyroscope via I2C.
 */
class MPU6050IMU : public IIMU {
public:
    void readSensor() override;
    void getGyroRates(float& rollRate, float& pitchRate, float& yawRate) const override;
    void getAccAngles(float& rollAngle, float& pitchAngle) const override;

    // Simulation/Override functionality
    void setOverride(float rollRate, float pitchRate, float yawRate,
                      float rollAngle, float pitchAngle) override;
    void setOverrideActive(bool active) override { overrideActive_ = active; }
    bool isOverrideActive() const override { return overrideActive_; }

private:
    float rollRate_ = 0.0f, pitchRate_ = 0.0f, yawRate_ = 0.0f;
    float rollAngle_ = 0.0f, pitchAngle_ = 0.0f;

    // Override states
    bool overrideActive_ = false;
    float oRollRate_ = 0.0f, oPitchRate_ = 0.0f, oYawRate_ = 0.0f;
    float oRollAngle_ = 0.0f, oPitchAngle_ = 0.0f;
};

#endif // MPU6050IMU_H
