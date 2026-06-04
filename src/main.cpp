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
        physicalBattery.update(); // sole writer of currentVoltage_ — Core 0 only
        
        bool isLow = physicalBattery.isLow();
        bool isArmed = physicalPpm.getChannel(FlightController::ARM_CHANNEL) > FlightController::ARM_THRESHOLD && !physicalPpm.isSignalLost();
        
        if (isLow) {
            // Low battery: rapid warning blink (100ms ON, 100ms OFF)
            digitalWrite(2, HIGH); delay(100);
            digitalWrite(2, LOW);  delay(100);
        } else if (isArmed) {
            // Armed/Fly state: Solid ON
            digitalWrite(2, HIGH);
            delay(100);
        } else {
            // Disarmed/Config state: Slow heartbeat pulse (50ms ON, 950ms OFF)
            digitalWrite(2, HIGH); delay(50);
            digitalWrite(2, LOW);  delay(950);
        }
    }
}

void flightControlTask(void *pvParameters) {
    constexpr uint32_t kPeriodUs = 4000; // 250Hz
    constexpr float    kDt       = kPeriodUs * 1e-6f;
    loopTimer = micros();
    while (1) {
        fc.update(kDt);
        while ((micros() - loopTimer) < kPeriodUs);
        loopTimer += kPeriodUs;
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
