#ifndef NATIVE_BUILD
#include <Arduino.h>
#include <Wire.h>
#include "hardware/MPU6500IMU.h"
#include "hardware/IBusReceiverDriver.h"
#include "hardware/PWMESP32Motors.h"
#include "hardware/ADCBatteryMonitor.h"
#include "hardware/QMC5883LCompass.h"
#include "hardware/ESP32LEDIndicator.h"
#include "network/WebDashboardServer.h"
#include "network/WebDashboardHandlers.h"
#include "core/FlightController.h"

MPU6500IMU physicalImu(5);
IBusReceiverDriver physicalPpm(&Serial2);
PWMESP32Motors physicalMotors(25, 27, 4, 14);
ADCBatteryMonitor physicalBattery(33, 3.3f, 77600.0f, 29400.0f);
ESP32LEDIndicator physicalIndicator(2);
QMC5883LCompass physicalCompass;
WebDashboardServer webServer;

FlightController fc(physicalImu, physicalPpm, physicalMotors, physicalBattery);
uint32_t loopTimer = 0;

void batteryMonitorTask(void *pvParameters) {
    physicalIndicator.init();
    while (1) {
        physicalBattery.update(); // sole writer of currentVoltage_ — Core 0 only
        
        physicalIndicator.setLowBattery(physicalBattery.isLow());
        physicalIndicator.setArmed(physicalPpm.getChannel(FlightController::ARM_CHANNEL) > FlightController::ARM_THRESHOLD && !physicalPpm.isSignalLost());
        physicalIndicator.update();
        
        delay(20); // 50Hz polling rate for smooth non-blocking execution
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
