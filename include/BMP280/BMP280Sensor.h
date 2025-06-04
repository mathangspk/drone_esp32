#ifndef BMP280_SENSOR_H
#define BMP280_SENSOR_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class BME280Sensor {
  public:
    BME280Sensor();
    bool begin();
    float readTemperature();
    float readPressure();
    float readHumidity();
  private:
    Adafruit_BME280 bme;
};

#endif