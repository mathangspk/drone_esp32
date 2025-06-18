#include "devices/MPU6500.h"

MPU6500::MPU6500(uint8_t csPin) : _cs(csPin) {}

void MPU6500::begin()
{
    SPI.begin(18, 19, 23, _cs);
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);

    writeRegister(0x6B, 0x00); // Wake up MPU-6500
    writeRegister(0x19, 0x00); // Max sample rate

    // Cấu hình gyroscope ±500°/s
    writeRegister(0x1B, 0x08); // 0000 1000b
    
    // Cấu hình accelerometer ±4g
    writeRegister(0x1C, 0x08); // 0000 1000b
}

bool MPU6500::isConnected()
{
    // Đọc WHO_AM_I register (0x75) - MPU6500 sẽ trả về 0x70
    uint8_t whoAmI = readRegister(0x75);
    return (whoAmI == 0x70);
}

SensorData MPU6500::getData()
{
    uint8_t buffer[14];
    SensorData data;

    readBytes(0x3B, buffer, 14); // Read accel, temp, gyro

    int16_t axRaw = (buffer[0] << 8) | buffer[1];
    int16_t ayRaw = (buffer[2] << 8) | buffer[3];
    int16_t azRaw = (buffer[4] << 8) | buffer[5];
    int16_t tempRaw = (buffer[6] << 8) | buffer[7];
    int16_t gxRaw = (buffer[8] << 8) | buffer[9];
    int16_t gyRaw = (buffer[10] << 8) | buffer[11];
    int16_t gzRaw = (buffer[12] << 8) | buffer[13];


      // Cho thang đo ±4g
    data.ax = axRaw / 8192.0;  // thay vì 16384.0
    data.ay = ayRaw / 8192.0;
    data.az = azRaw / 8192.0;

    data.temp = (tempRaw / 340.0) + 36.53;
    // Cho thang đo ±500°/s
    data.gx = gxRaw / 65.5;    // thay vì 131.0
    data.gy = gyRaw / 65.5;
    data.gz = gzRaw / 65.5;
    /*
    data.ax = axRaw / 16384.0;
    data.ay = ayRaw / 16384.0;
    data.az = azRaw / 16384.0;
    data.temp = (tempRaw / 340.0) + 36.53;
    data.gx = gxRaw / 131.0;
    data.gy = gyRaw / 131.0;
    data.gz = gzRaw / 131.0;
 */
    return data;
}
void MPU6500::readSensor()
{
    uint8_t buffer[14];
    burstRead(0x3B, buffer, 14);

    int16_t ax = (buffer[0] << 8) | buffer[1];
    int16_t ay = (buffer[2] << 8) | buffer[3];
    int16_t az = (buffer[4] << 8) | buffer[5];
    int16_t tempRaw = (buffer[6] << 8) | buffer[7];
    int16_t gx = (buffer[8] << 8) | buffer[9];
    int16_t gy = (buffer[10] << 8) | buffer[11];
    int16_t gz = (buffer[12] << 8) | buffer[13];

    accelX = ax / 8192.0;
    accelY = ay / 8192.0;
    accelZ = az / 8192.0;
    temperature = (tempRaw / 340.0) + 36.53;
    gyroX = gx / 65.5;
    gyroY = gy / 65.5;
    gyroZ = gz / 65.5;
    /*
    accelX = ax / 16384.0;
    accelY = ay / 16384.0;
    accelZ = az / 16384.0;
    temperature = (tempRaw / 340.0) + 36.53;
    gyroX = gx / 131.0;
    gyroY = gy / 131.0;
    gyroZ = gz / 131.0;

    */
}

float MPU6500::getAccelX() const { return accelX; }
float MPU6500::getAccelY() const { return accelY; }
float MPU6500::getAccelZ() const { return accelZ; }
float MPU6500::getGyroX() const { return gyroX; }
float MPU6500::getGyroY() const { return gyroY; }
float MPU6500::getGyroZ() const { return gyroZ; }
float MPU6500::getTemperature() const { return temperature; }

void MPU6500::writeRegister(uint8_t reg, uint8_t value)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    SPI.transfer(reg & 0x7F);
    SPI.transfer(value);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

uint8_t MPU6500::readRegister(uint8_t reg)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    SPI.transfer(reg | 0x80);
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    return val;
}

void MPU6500::burstRead(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    SPI.transfer(reg | 0x80);
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

uint8_t MPU6500::readByte(uint8_t reg)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    SPI.transfer(reg | 0x80); // MSB = 1 for read
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    return val;
}

void MPU6500::readBytes(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs, LOW);
    SPI.transfer(reg | 0x80);
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}
