#ifndef NATIVE_BUILD
#include <Arduino.h>
#include <Wire.h>
#include "hardware/MPU6500IMU.h"
#include "hardware/IBusReceiverDriver.h"
#include "hardware/PWMESP32Motors.h"
#include "hardware/ADCBatteryMonitor.h"
#include "hardware/QMC5883LCompass.h"
#include "network/WebDashboardServer.h"
#include "network/WebDashboardHandlers.h"
#include "core/FlightController.h"

MPU6500IMU physicalImu(5);
IBusReceiverDriver physicalPpm(&Serial2);
PWMESP32Motors physicalMotors(25, 27, 4, 14);
ADCBatteryMonitor physicalBattery(33, 3.3f, 77600.0f, 29400.0f);
QMC5883LCompass physicalCompass;
WebDashboardServer webServer;

FlightController fc(physicalImu, physicalPpm, physicalMotors, physicalBattery);
uint32_t loopTimer = 0;

void batteryMonitorTask(void *pvParameters) {
    pinMode(2, OUTPUT);
    while (1) {
        float voltage = physicalBattery.readVoltage();
        if (physicalBattery.isLow()) {
            digitalWrite(2, HIGH); delay(500);
            digitalWrite(2, LOW); delay(500);
        } else {
            digitalWrite(2, HIGH);
            delay(1000);
        }
    }
}

void flightControlTask(void *pvParameters) {
    loopTimer = micros();
    while (1) {
        fc.update(0.004f);
        while ((micros() - loopTimer) < 4000);
        loopTimer = micros();
    }
}

void webDashboardTask(void *pvParameters) {
    WebDashboardHandlers::init(physicalPpm, physicalMotors, physicalBattery, physicalImu);
    while (1) {
        if (physicalPpm.getChannel(4) > 1500) {
            webServer.stop();
            delay(500);
        } else {
            webServer.begin();
            webServer.handleClient();
            delay(5);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(250);

    physicalImu.begin();
    physicalCompass.begin();
    physicalMotors.init();
    physicalBattery.init();
    physicalPpm.begin();
    fc.init();

    xTaskCreatePinnedToCore(batteryMonitorTask, "Battery Task", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(webDashboardTask, "Web Task", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(flightControlTask, "Flight Task", 8192, NULL, 2, NULL, 1);
}

void loop() {}
#endif // NATIVE_BUILD
