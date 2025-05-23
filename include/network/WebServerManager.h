#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <controllers/ESCControllers.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <battery/BatteryMonitor.h>
#include <currentMeasure/CurrentMeasure.h>


class WebServerManager
{
public:
    WebServerManager(const char *ssid, const char *password, ESCController &escController, StatusLED &led, MPU6500 &mpu6500, BatteryMonitor &battery, CurrentMonitor &current);
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
    void setupRoutes();
};

#endif