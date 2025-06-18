#include "../../include/BME280/BME280Sensor.h"

BME280Sensor::BME280Sensor() {}

bool BME280Sensor::begin() {
    return bme.begin(0x76); // Địa chỉ I2C mặc định của BME280
}

float BME280Sensor::readTemperature() {
    return bme.readTemperature();
}

float BME280Sensor::readPressure() {
    return bme.readPressure() / 100.0f; // Chuyển từ Pa sang hPa
}

float BME280Sensor::readHumidity() {
    return bme.readHumidity();
}
