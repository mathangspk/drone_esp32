#ifndef MPU6500_H
#define MPU6500_H

#include <Arduino.h>
#include <SPI.h>

struct SensorData
{
    float ax, ay, az; // Accelerometer (g)
    float gx, gy, gz; // Gyroscope (deg/s)
    float temp;       // Temperature (°C)
};
class MPU6500
{
public:
    MPU6500(uint8_t csPin);
    void begin();
    bool isConnected();
    void readSensor();
    SensorData getData();
    float getAccelX() const;
    float getAccelY() const;
    float getAccelZ() const;
    float getGyroX() const;
    float getGyroY() const;
    float getGyroZ() const;
    float getTemperature() const;

private:
    uint8_t _cs;
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    void burstRead(uint8_t reg, uint8_t *buffer, uint8_t length);
    uint8_t readByte(uint8_t reg);
    void readBytes(uint8_t reg, uint8_t *buffer, uint8_t length);
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float temperature;
};

#endif
