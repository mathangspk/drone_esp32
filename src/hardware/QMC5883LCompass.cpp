#include "hardware/QMC5883LCompass.h"
#include <cmath>

#ifndef NATIVE_BUILD
#include <Wire.h>
#endif

QMC5883LCompass::QMC5883LCompass() {}

bool QMC5883LCompass::begin() {
#ifndef NATIVE_BUILD
    if (!writeReg(0x0A, 0x80)) return false; // Reset
    delay(10);
    if (!writeReg(0x0B, 0x01)) return false; // Set/Reset Period
    if (!writeReg(0x09, 0x0D)) return false; // Continuous mode, 50Hz ODR, 2G Range, 512 OSR
    delay(50);
#endif
    return true;
}

void QMC5883LCompass::readMag(float &mx, float &my, float &mz) {
    mx = 0.0f; my = 0.0f; mz = 0.0f;
#ifndef NATIVE_BUILD
    uint8_t status = 0;
    readRegs(0x06, &status, 1);
    if (!(status & 0x01)) return; // Data not ready
    uint8_t buf[6];
    if (!readRegs(0x00, buf, 6)) return;
    int16_t x = (int16_t)(buf[1] << 8 | buf[0]);
    int16_t y = (int16_t)(buf[3] << 8 | buf[2]);
    int16_t z = (int16_t)(buf[5] << 8 | buf[4]);

    // Hard-iron offset subtracted first, then soft-iron scale applied
    mx = (static_cast<float>(x) - offset_[0]) * scale_[0] * (2.0f / 32768.0f);
    my = (static_cast<float>(y) - offset_[1]) * scale_[1] * (2.0f / 32768.0f);
    mz = (static_cast<float>(z) - offset_[2]) * scale_[2] * (2.0f / 32768.0f);
#endif
}

float QMC5883LCompass::getHeading() {
    float mx = 0, my = 0, mz = 0;
    readMag(mx, my, mz);
    float heading = std::atan2(my, mx) * (180.0f / 3.14159f);
    if (heading < 0) heading += 360.0f;
    return heading;
}

void QMC5883LCompass::setCalibration(float ox, float oy, float oz, float sx, float sy, float sz) {
    offset_[0] = ox; offset_[1] = oy; offset_[2] = oz;
    scale_[0] = sx; scale_[1] = sy; scale_[2] = sz;
}

#ifndef NATIVE_BUILD
bool QMC5883LCompass::writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool QMC5883LCompass::readRegs(uint8_t reg, uint8_t *buf, uint8_t count) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(ADDR, count) != count) return false;
    for (uint8_t i = 0; i < count && Wire.available(); i++) {
        buf[i] = Wire.read();
    }
    return true;
}
#else
bool QMC5883LCompass::writeReg(uint8_t, uint8_t) { return true; }
bool QMC5883LCompass::readRegs(uint8_t, uint8_t*, uint8_t) { return true; }
#endif
