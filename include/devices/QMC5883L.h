#ifndef QMC5883L_H
#define QMC5883L_H

#include <Arduino.h>
#include <Wire.h>

class QMC5883L
{
public:
    QMC5883L();
    bool begin();
    void readMag(float &mx, float &my, float &mz);
    float getHeading();
    void calibrate();
    bool dataReady();

private:
    static const uint8_t QMC5883L_ADDR = 0x0D; // Địa chỉ I2C mặc định

    // Các thanh ghi
    static const uint8_t REG_DATA_X_LSB = 0x00;
    static const uint8_t REG_DATA_X_MSB = 0x01;
    static const uint8_t REG_DATA_Y_LSB = 0x02;
    static const uint8_t REG_DATA_Y_MSB = 0x03;
    static const uint8_t REG_DATA_Z_LSB = 0x04;
    static const uint8_t REG_DATA_Z_MSB = 0x05;
    static const uint8_t REG_STATUS = 0x06;
    static const uint8_t REG_TEMP_LSB = 0x07;
    static const uint8_t REG_TEMP_MSB = 0x08;
    static const uint8_t REG_CONTROL1 = 0x09;
    static const uint8_t REG_CONTROL2 = 0x0A;
    static const uint8_t REG_SET_RESET = 0x0B;

    // Các bit cấu hình
    static const uint8_t MODE_CONTINUOUS = 0x01; // 01: Continuous mode
    static const uint8_t ODR_10HZ = 0x00;        // 00: 10Hz
    static const uint8_t ODR_50HZ = 0x04;        // 01: 50Hz
    static const uint8_t ODR_100HZ = 0x08;       // 10: 100Hz
    static const uint8_t ODR_200HZ = 0x0C;       // 11: 200Hz

    static const uint8_t RNG_2G = 0x00; // 00: ±2G
    static const uint8_t RNG_8G = 0x10; // 01: ±8G

    static const uint8_t OSR_512 = 0x00; // 00: 512 (lowest noise)
    static const uint8_t OSR_256 = 0x40; // 01: 256
    static const uint8_t OSR_128 = 0x80; // 10: 128
    static const uint8_t OSR_64 = 0xC0;  // 11: 64 (highest speed)

    // Các biến hiệu chuẩn
    float mag_offset[3] = {0, 0, 0}; // offset cho x, y, z
    float mag_scale[3] = {1, 1, 1};  // scale cho x, y, z
    float mag_range = 2.0f;          // Thang đo mặc định ±2G

    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t count);
    int16_t readRawAxis(uint8_t lsbRegister, uint8_t msbRegister);
    void setRange(uint8_t range);
    void setOSR(uint8_t osr);
    void setODR(uint8_t odr);
};

#endif