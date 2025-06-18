#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <controllers/ESCControllers.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <devices/battery/BatteryMonitor.h>
#include <devices/currentMeasure/CurrentMeasure.h>
#include <BME280/BME280Sensor.h>
#include <orientation/SimpleMahony.h>
#include <controllers/DronePIDController.h>
#include <receiver/IBusReceiver.h>

class WebServerManager
{
public:
    WebServerManager(const char *ssid, const char *password, 
                    ESCController &escController, StatusLED &led, 
                    MPU6500 &mpu6500, BatteryMonitor &battery, 
                    CurrentMonitor &current, BME280Sensor &bmeSensor,
                    SimpleMahony &mahony, DronePIDController &pidController,
                    IBusReceiver &receiver);
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
    SimpleMahony &_mahony;
    DronePIDController &_pidController;
    IBusReceiver &_receiver;
    void setupRoutes();
};

#endif