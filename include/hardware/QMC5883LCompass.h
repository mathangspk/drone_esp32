#ifndef QMC5883LCOMPASS_H
#define QMC5883LCOMPASS_H

#include <Arduino.h>

/**
 * @brief Companion driver for the QMC5883L I2C magnetometer.
 * Provides heading measurements and calibration storage.
 */
class QMC5883LCompass {
public:
    QMC5883LCompass();
    bool begin();
    void readMag(float &mx, float &my, float &mz);
    float getHeading();
    void setCalibration(float ox, float oy, float oz, float sx, float sy, float sz);

private:
    static const uint8_t ADDR = 0x0D;
    float offset_[3] = {0.0f, 0.0f, 0.0f};
    float scale_[3] = {1.0f, 1.0f, 1.0f};

    bool writeReg(uint8_t reg, uint8_t val);
    bool readRegs(uint8_t reg, uint8_t *buf, uint8_t count);
};

#endif // QMC5883LCOMPASS_H
