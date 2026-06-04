#include "network/WebDashboardHandlers.h"
#include "core/FlightController.h"

void WebDashboardHandlers::handleMotorTest(WebServer& server) {
    if (!ppm_ || !motors_) { server.send(500, "text/plain", "Not initialized"); return; }
    bool act = server.arg("active") == "true";
    int idx = server.arg("motorIdx").toInt();
    int val = server.arg("value").toInt();
    if (val > 1150) val = 1150;
    if (val < 1000) val = 1000;
    if (act && ppm_->getChannel(FlightController::ARM_CHANNEL) > FlightController::ARM_THRESHOLD) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Cannot override: Transmitter is ARMED!\"}");
        return;
    }
    if (idx == -1) {
        for (int i = 0; i < 4; i++) motors_->setOverride(i, 1000, false);
    } else if (idx >= 0 && idx < 4) {
        motors_->setOverride(idx, val, act);
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

void WebDashboardHandlers::handleESCCalibration(WebServer& server) {
    if (!ppm_ || !motors_) { server.send(500, "text/plain", "Not initialized"); return; }
    if (ppm_->getChannel(FlightController::ARM_CHANNEL) > FlightController::ARM_THRESHOLD) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Cannot calibrate: Transmitter is ARMED!\"}");
        return;
    }
    String step = server.arg("step");
    if (step == "high") {
        for (int i = 0; i < 4; ++i) motors_->setOverride(i, 2000, true);
    } else if (step == "low") {
        for (int i = 0; i < 4; ++i) motors_->setOverride(i, 1000, true);
    } else if (step == "stop") {
        for (int i = 0; i < 4; ++i) motors_->setOverride(i, 1000, false);
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

void WebDashboardHandlers::handleGetIMU(WebServer& server) {
    if (!imu_) { server.send(500, "text/plain", "Not initialized"); return; }
    imu_->readSensor();
    float aRoll = 0, aPitch = 0;
    float gRoll = 0, gPitch = 0, gYaw = 0;
    imu_->getAccAngles(aRoll, aPitch);
    imu_->getGyroRates(gRoll, gPitch, gYaw);
    char buf[128];
    snprintf(buf, sizeof(buf), "{\"a_r\":%.1f,\"a_p\":%.1f,\"g_r\":%.1f,\"g_p\":%.1f,\"g_y\":%.1f}",
             aRoll, aPitch, gRoll, gPitch, gYaw);
    server.send(200, "application/json", buf);
}
