#include <Arduino.h>
#include <controllers/ESCControllers.h>
#include <network/WebServerManager.h>
#include <systemState/StatusLED.h>
#include <devices/MPU6500.h>
#include <battery/BatteryMonitor.h>

#define STATUS_LED_PIN 2

const char *ssid = "ESP32-WIFI";
const char *password = "12345678";

ESCController escController;
StatusLED statusLed(STATUS_LED_PIN);
MPU6500 mpu(5);
BatteryMonitor battery(33);
WebServerManager webServer(ssid, password, escController, statusLed, mpu, battery);

void setup()
{
  Serial.begin(115200);

  statusLed.begin();
  statusLed.blink(200, 3); // Nháy 3 lần khi khởi động

  mpu.begin();
  battery.begin();

  escController.begin();

  webServer.begin();

  statusLed.on();
}

void loop()
{
  // statusLed.blink(2000, 100);
  webServer.handleClient();
  // battery.readVoltage();
  // delay(1000);
}
