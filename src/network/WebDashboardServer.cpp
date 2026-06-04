#include "network/WebDashboardServer.h"
#include "network/WebDashboardHandlers.h"

#ifndef NATIVE_BUILD
WebDashboardServer::WebDashboardServer() : server_(80) {}

void WebDashboardServer::registerRoutes() {
    server_.on("/", HTTP_GET, [this]() { WebDashboardHandlers::handleRoot(this->server_); });
    server_.on("/api/pid", HTTP_GET, [this]() { WebDashboardHandlers::handleGetPID(this->server_); });
    server_.on("/api/pid", HTTP_POST, [this]() { WebDashboardHandlers::handleSetPID(this->server_); });
    server_.on("/api/receiver", HTTP_GET, [this]() { WebDashboardHandlers::handleGetReceiver(this->server_); });
    server_.on("/api/receiver", HTTP_POST, [this]() { WebDashboardHandlers::handleSetReceiver(this->server_); });
    server_.on("/api/motor", HTTP_POST, [this]() { WebDashboardHandlers::handleMotorTest(this->server_); });
    server_.on("/api/calibrate", HTTP_POST, [this]() { WebDashboardHandlers::handleESCCalibration(this->server_); });
    server_.on("/api/imu", HTTP_GET, [this]() { WebDashboardHandlers::handleGetIMU(this->server_); });
    server_.on("/api/log", HTTP_GET, [this]() { WebDashboardHandlers::handleGetLog(this->server_); });
    routesRegistered_ = true;
}

void WebDashboardServer::begin() {
    if (isRunning_) return;
    WiFi.softAP("ESP32_Drone_Config", "12345678");
    delay(100);
    if (!routesRegistered_) registerRoutes();
    server_.begin();
    isRunning_ = true;
    Serial.println("Web Dashboard server started (SoftAP: ESP32_Drone_Config)");
}

void WebDashboardServer::handleClient() {
    if (isRunning_) {
        server_.handleClient();
    }
}

void WebDashboardServer::stop() {
    if (!isRunning_) return;
    server_.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    isRunning_ = false;
    Serial.println("Web Dashboard server stopped. Wi-Fi powered down.");
}
#else
WebDashboardServer::WebDashboardServer() {}
void WebDashboardServer::begin() {}
void WebDashboardServer::handleClient() {}
void WebDashboardServer::stop() {}
#endif
