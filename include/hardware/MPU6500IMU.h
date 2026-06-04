#ifndef MPU6500IMU_H
#define MPU6500IMU_H

#include "interfaces/IIMU.h"
#include <Arduino.h>

/**
 * @brief SPI hardware driver for the MPU6500 IMU, implementing the abstract IIMU interface.
 */
class MPU6500IMU : public IIMU {
public:
    MPU6500IMU(uint8_t csPin);
    void begin();
    bool isConnected();

    void readSensor() override;
    void getGyroRates(float& rollRate, float& pitchRate, float& yawRate) const override;
    void getAccAngles(float& rollAngle, float& pitchAngle) const override;

    void setOverride(float rollRate, float pitchRate, float yawRate,
                      float rollAngle, float pitchAngle) override;
    void setOverrideActive(bool active) override { oActive_ = active; }
    bool isOverrideActive() const override { return oActive_; }

private:
    static constexpr float GYRO_SCALE  = 65.5f;   // LSB/(deg/s) for ±500 dps (reg 0x1B=0x08)
    static constexpr float ACCEL_SCALE = 8192.0f;  // LSB/g for ±4 g            (reg 0x1C=0x08)

    uint8_t cs_;
    float rollRate_ = 0.0f, pitchRate_ = 0.0f, yawRate_ = 0.0f;
    float rollAngle_ = 0.0f, pitchAngle_ = 0.0f;

    // Simulation overrides
    bool oActive_ = false;
    float oRollRate_ = 0.0f, oPitchRate_ = 0.0f, oYawRate_ = 0.0f;
    float oRollAngle_ = 0.0f, oPitchAngle_ = 0.0f;

#ifndef NATIVE_BUILD
    void writeReg(uint8_t reg, uint8_t val);
    uint8_t readReg(uint8_t reg);
    void readBytes(uint8_t reg, uint8_t *buf, uint8_t len);
#else
    void writeReg(uint8_t, uint8_t) {}
    uint8_t readReg(uint8_t) { return 0; }
    void readBytes(uint8_t, uint8_t*, uint8_t) {}
#endif
};

#endif // MPU6500IMU_H
