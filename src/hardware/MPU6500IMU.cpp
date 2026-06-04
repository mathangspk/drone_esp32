#include "hardware/MPU6500IMU.h"
#include <cmath>

#ifndef NATIVE_BUILD
#include <SPI.h>
#endif

#ifndef NATIVE_BUILD
MPU6500IMU::MPU6500IMU(uint8_t csPin) : cs_(csPin) {}

void MPU6500IMU::begin() {
    SPI.begin(18, 19, 23, cs_);
    pinMode(cs_, OUTPUT);
    digitalWrite(cs_, HIGH);
    delay(10);
    writeReg(0x6B, 0x80); // Device reset
    delay(100);            // Wait for reset to complete
    writeReg(0x6B, 0x00); // Wake up (internal 20MHz clock)
    delay(100);            // Wait for oscillator to stabilize
    writeReg(0x19, 0x00); // Sample rate divider = 0 (max rate)
    writeReg(0x1A, 0x03); // DLPF_CFG=3: Gyro BW 41Hz, 5.9ms delay
    writeReg(0x1B, 0x08); // Gyro ±500dps
    writeReg(0x1C, 0x08); // Accel ±4g
}

bool MPU6500IMU::isConnected() {
    return readReg(0x75) == 0x70;
}

void MPU6500IMU::readSensor() {
    if (oActive_) return;
    uint8_t buffer[14];
    readBytes(0x3B, buffer, 14);
    int16_t ax = (buffer[0] << 8) | buffer[1];
    int16_t ay = (buffer[2] << 8) | buffer[3];
    int16_t az = (buffer[4] << 8) | buffer[5];
    int16_t gx = (buffer[8] << 8) | buffer[9];
    int16_t gy = (buffer[10] << 8) | buffer[11];
    int16_t gz = (buffer[12] << 8) | buffer[13];

    rollRate_ = static_cast<float>(gx) / GYRO_SCALE;
    pitchRate_ = static_cast<float>(gy) / GYRO_SCALE;
    yawRate_ = static_cast<float>(gz) / GYRO_SCALE;

    float accX = static_cast<float>(ax) / ACCEL_SCALE;
    float accY = static_cast<float>(ay) / ACCEL_SCALE;
    float accZ = static_cast<float>(az) / ACCEL_SCALE;

    rollAngle_  =  std::atan2(accY, std::sqrt(accX * accX + accZ * accZ)) * kRadToDeg;
    pitchAngle_ = -std::atan2(accX, std::sqrt(accY * accY + accZ * accZ)) * kRadToDeg;
}
#else
MPU6500IMU::MPU6500IMU(uint8_t csPin) : cs_(csPin) {}
void MPU6500IMU::begin() {}
bool MPU6500IMU::isConnected() { return true; }
void MPU6500IMU::readSensor() {}
#endif

void MPU6500IMU::getGyroRates(float& r, float& p, float& y) const {
    r = oActive_ ? oRollRate_ : rollRate_;
    p = oActive_ ? oPitchRate_ : pitchRate_;
    y = oActive_ ? oYawRate_ : yawRate_;
}

void MPU6500IMU::getAccAngles(float& r, float& p) const {
    r = oActive_ ? oRollAngle_ : rollAngle_;
    p = oActive_ ? oPitchAngle_ : pitchAngle_;
}

void MPU6500IMU::setOverride(float rR, float pR, float yR, float rA, float pA) {
    oRollRate_ = rR; oPitchRate_ = pR; oYawRate_ = yR;
    oRollAngle_ = rA; oPitchAngle_ = pA;
}

#ifndef NATIVE_BUILD
void MPU6500IMU::writeReg(uint8_t reg, uint8_t val) {
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(cs_, LOW);
    SPI.transfer(reg & 0x7F);
    SPI.transfer(val);
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();
}

uint8_t MPU6500IMU::readReg(uint8_t reg) {
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(cs_, LOW);
    SPI.transfer(reg | 0x80);
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();
    return val;
}

void MPU6500IMU::readBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(cs_, LOW);
    SPI.transfer(reg | 0x80);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = SPI.transfer(0x00);
    }
    digitalWrite(cs_, HIGH);
    SPI.endTransaction();
}
#endif
