#include "network/WebDashboardHandlers.h"
#include "network/WebDashboardPage.h"
#include "core/FlightController.h"
#ifndef NATIVE_BUILD
#include <Preferences.h>
#endif

IPPM* WebDashboardHandlers::ppm_ = nullptr;
IMotors* WebDashboardHandlers::motors_ = nullptr;
IBattery* WebDashboardHandlers::battery_ = nullptr;
IIMU* WebDashboardHandlers::imu_ = nullptr;

void WebDashboardHandlers::init(IPPM& ppm, IMotors& motors, IBattery& battery, IIMU& imu) {
    ppm_ = &ppm; motors_ = &motors; battery_ = &battery; imu_ = &imu;
#ifndef NATIVE_BUILD
    if (!logMutex_) logMutex_ = xSemaphoreCreateMutex();
#endif
    clearFlightLog();
}

void WebDashboardHandlers::handleRoot(WebServer& server) {
    server.send(200, "text/html", kDashboardHTML);
}

void WebDashboardHandlers::handleGetPID(WebServer& server) {
    float r_kp=FlightController::kDefaultRateKp,  r_ki=FlightController::kDefaultRateKi,  r_kd=FlightController::kDefaultRateKd;
    float p_kp=FlightController::kDefaultRateKp,  p_ki=FlightController::kDefaultRateKi,  p_kd=FlightController::kDefaultRateKd;
    float y_kp=FlightController::kDefaultYawKp,   y_ki=FlightController::kDefaultYawKi,   y_kd=FlightController::kDefaultYawKd;
    float ra_kp=FlightController::kDefaultAngleKp, ra_kd=FlightController::kDefaultAngleKd;
    float pa_kp=FlightController::kDefaultAngleKp, pa_kd=FlightController::kDefaultAngleKd;
#ifndef NATIVE_BUILD
    Preferences prefs;
    prefs.begin("pid", true);
    r_kp = prefs.getFloat("r_kp", r_kp); r_ki = prefs.getFloat("r_ki", r_ki); r_kd = prefs.getFloat("r_kd", r_kd);
    p_kp = prefs.getFloat("p_kp", p_kp); p_ki = prefs.getFloat("p_ki", p_ki); p_kd = prefs.getFloat("p_kd", p_kd);
    y_kp = prefs.getFloat("y_kp", y_kp); y_ki = prefs.getFloat("y_ki", y_ki); y_kd = prefs.getFloat("y_kd", y_kd);
    ra_kp = prefs.getFloat("ra_kp", ra_kp); ra_kd = prefs.getFloat("ra_kd", ra_kd);
    pa_kp = prefs.getFloat("pa_kp", pa_kp); pa_kd = prefs.getFloat("pa_kd", pa_kd);
    prefs.end();
#endif
    char buf[256];
    snprintf(buf, sizeof(buf), "{\"r_kp\":%.3f,\"r_ki\":%.3f,\"r_kd\":%.3f,\"p_kp\":%.3f,\"p_ki\":%.3f,\"p_kd\":%.3f,\"y_kp\":%.3f,\"y_ki\":%.3f,\"y_kd\":%.3f,\"ra_kp\":%.1f,\"ra_kd\":%.1f,\"pa_kp\":%.1f,\"pa_kd\":%.1f}",
             r_kp, r_ki, r_kd, p_kp, p_ki, p_kd, y_kp, y_ki, y_kd, ra_kp, ra_kd, pa_kp, pa_kd);
    server.send(200, "application/json", buf);
}

void WebDashboardHandlers::handleSetPID(WebServer& server) {
#ifndef NATIVE_BUILD
    Preferences prefs;
    prefs.begin("pid", false);
    const char* keys[] = {"r_kp", "r_ki", "r_kd", "p_kp", "p_ki", "p_kd", "y_kp", "y_ki", "y_kd", "ra_kp", "ra_kd", "pa_kp", "pa_kd"};
    for (const char* k : keys) {
        String val = server.arg(k);
        if (val.length() == 0) continue;
        float v = val.toFloat();
        if (v >= 0.0f && v <= 20.0f) prefs.putFloat(k, v); // reject out-of-range gains
    }
    prefs.end();
#endif
    server.send(200, "application/json", "{\"status\":\"success\"}");
}

void WebDashboardHandlers::handleGetReceiver(WebServer& server) {
    if (!ppm_) { server.send(500, "text/plain", "Not initialized"); return; }
    char buf[128];
    snprintf(buf, sizeof(buf), "{\"channels\":[%d,%d,%d,%d,%d,%d]}",
             ppm_->getChannel(0), ppm_->getChannel(1), ppm_->getChannel(2),
             ppm_->getChannel(3), ppm_->getChannel(4), ppm_->getChannel(5));
    server.send(200, "application/json", buf);
}

void WebDashboardHandlers::handleSetReceiver(WebServer& server) {
    if (!ppm_) { server.send(500, "text/plain", "Not initialized"); return; }
    bool act = server.arg("active") == "true";
    if (act && ppm_->getChannel(4) > 1500) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Cannot override: Transmitter is ARMED!\"}");
        return;
    }
    int idx = server.arg("channelIdx").toInt();
    int val = server.arg("value").toInt();
    ppm_->setOverrideActive(act);
    if (act && idx >= 0 && idx < 6) {
        if (val >= 1000 && val <= 2000) ppm_->setOverride(idx, val);
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

void WebDashboardHandlers::handleMotorTest(WebServer& server) {
    if (!ppm_ || !motors_) { server.send(500, "text/plain", "Not initialized"); return; }
    bool act = server.arg("active") == "true";
    int idx = server.arg("motorIdx").toInt();
    int val = server.arg("value").toInt();
    if (val > 1150) val = 1150;
    if (val < 1000) val = 1000;
    if (act && ppm_->getChannel(4) > 1500) {
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
