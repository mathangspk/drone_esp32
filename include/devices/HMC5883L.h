#ifndef HMC5883L_H
#define HMC5883L_H

#include <Arduino.h>
#include <Wire.h>

class HMC5883L {
public:
    HMC5883L();
    bool begin();
    void readMag(float &mx, float &my, float &mz);
    float getHeading();  // Trả về góc heading (yaw) tính từ từ trường
    void calibrate();    // Hiệu chuẩn offset và scale
    
private:
    uint8_t _address;    // Địa chỉ I2C của cảm biến
    
    // Các thanh ghi
    static const uint8_t CONFIG_A = 0x00;
    static const uint8_t CONFIG_B = 0x01;
    static const uint8_t MODE = 0x02;
    static const uint8_t DATA_X_MSB = 0x03;
    
    // Các biến hiệu chuẩn
    float mag_offset[3] = {0, 0, 0};  // offset cho x, y, z
    float mag_scale[3] = {1, 1, 1};   // scale cho x, y, z
    
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t count);
};

#endif 