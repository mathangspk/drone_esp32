#include "hardware/MPU6050IMU.h"
#include <cmath>

#ifndef NATIVE_BUILD
#include <Wire.h>
#include <Arduino.h>
#endif

void MPU6050IMU::readSensor() {
    if (overrideActive_) return;

#ifndef NATIVE_BUILD
    // Read accelerometer LSB
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    if (Wire.endTransmission() != 0) return;
    
    if (Wire.requestFrom(0x68, 6) != 6) return;
    int16_t accXLSB = Wire.read() << 8 | Wire.read();
    int16_t accYLSB = Wire.read() << 8 | Wire.read();
    int16_t accZLSB = Wire.read() << 8 | Wire.read();

    // Read gyroscope LSB
    Wire.beginTransmission(0x68);
    Wire.write(0x43);
    if (Wire.endTransmission() != 0) return;

    if (Wire.requestFrom(0x68, 6) != 6) return;
    int16_t gyroX = Wire.read() << 8 | Wire.read();
    int16_t gyroY = Wire.read() << 8 | Wire.read();
    int16_t gyroZ = Wire.read() << 8 | Wire.read();

    // Scale calculations matching sample code
    rollRate_ = static_cast<float>(gyroX) / 65.5f;
    pitchRate_ = static_cast<float>(gyroY) / 65.5f;
    yawRate_ = static_cast<float>(gyroZ) / 65.5f;

    float accX = static_cast<float>(accXLSB) / 4096.0f - 0.02f;
    float accY = static_cast<float>(accYLSB) / 4096.0f;
    float accZ = static_cast<float>(accZLSB) / 4096.0f - 0.08f;

    rollAngle_ = std::atan(accY / std::sqrt(accX * accX + accZ * accZ)) * (180.0f / 3.142f);
    pitchAngle_ = -std::atan(accX / std::sqrt(accY * accY + accZ * accZ)) * (180.0f / 3.142f);
#endif
}

void MPU6050IMU::getGyroRates(float& rollRate, float& pitchRate, float& yawRate) const {
    if (overrideActive_) {
        rollRate = oRollRate_; pitchRate = oPitchRate_; yawRate = oYawRate_;
    } else {
        rollRate = rollRate_; pitchRate = pitchRate_; yawRate = yawRate_;
    }
}

void MPU6050IMU::getAccAngles(float& rollAngle, float& pitchAngle) const {
    if (overrideActive_) {
        rollAngle = oRollAngle_; pitchAngle = oPitchAngle_;
    } else {
        rollAngle = rollAngle_; pitchAngle = pitchAngle_;
    }
}

void MPU6050IMU::setOverride(float rollRate, float pitchRate, float yawRate,
                              float rollAngle, float pitchAngle) {
    oRollRate_ = rollRate; oPitchRate_ = pitchRate; oYawRate_ = yawRate;
    oRollAngle_ = rollAngle; oPitchAngle_ = pitchAngle;
}
