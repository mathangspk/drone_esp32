#include <Arduino.h>
#include <controllers/ESCControllers.h>
#include <network/WebServerManager.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <battery/BatteryMonitor.h>
#include <currentMeasure/CurrentMeasure.h>

#define STATUS_LED_PIN 2

const char *ssid = "ESP32-WIFI";
const char *password = "12345678";

ESCController escController;
StatusLED statusLed(STATUS_LED_PIN);
MPU6500 mpu(5);
BatteryMonitor battery(33);
CurrentMonitor current(34);
WebServerManager webServer(ssid, password, escController, statusLed, mpu, battery, current);

void setup()
{
  Serial.begin(115200);

  statusLed.begin();
  statusLed.blink(200, 3); // Nháy 3 lần khi khởi động

  mpu.begin();
  battery.begin();
  current.begin();

  escController.begin();

  webServer.begin();

  statusLed.on();
}

void loop()
{
  // statusLed.blink(2000, 100);
  webServer.handleClient();
   //battery.readVoltage();
   //current.readVoltage();
   //delay(1000);
}
