#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESCcontrollers/ESCControllers.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <devices/battery/BatteryMonitor.h>
#include <devices/currentMeasure/CurrentMeasure.h>
#include <BMP280/BMP280Sensor.h>

class WebServerManager
{
public:
    WebServerManager(const char *ssid, const char *password, ESCController &escController, StatusLED &led, MPU6500 &mpu6500, BatteryMonitor &battery, 
        CurrentMonitor &current, BME280Sensor &bmeSensor);
    void begin();
    void handleClient();

private:
    const char *_ssid;
    const char *_password;
    WebServer server;
    ESCController &_esc;
    StatusLED &_statusLED;
    MPU6500 &_mpu6500;
    BatteryMonitor &_battery;
    CurrentMonitor &_current;
    BME280Sensor &_bme280;
    void setupRoutes();
};

#endif