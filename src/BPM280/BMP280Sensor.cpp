
#include "BMP280/BMP280Sensor.h"

BME280Sensor::BME280Sensor() {}

bool BME280Sensor::begin() {
  
  return bme.begin(0x76); // địa chỉ mặc định là 0x76 hoặc 0x77 tùy module
}

float BME280Sensor::readTemperature() {
    float temp = bme.readTemperature();
    //Serial.printf("BMP280 Temp: %.3f C\n", temp);
  return bme.readTemperature();
}

float BME280Sensor::readPressure() {
    float pressure = bme.readPressure();
    //Serial.printf("BMP280 Pressure: %.3f C\n", pressure);
    uint8_t chip_id = bme.sensorID(); // Hàm có sẵn
  //Serial.print("BME280 Chip ID: 0x");
  //Serial.println(chip_id, HEX);  // Thường là 0x60
  return bme.readPressure() / 100.0; // hPa
}
