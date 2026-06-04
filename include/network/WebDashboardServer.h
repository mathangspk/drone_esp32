#ifndef WEBDASHBOARDSERVER_H
#define WEBDASHBOARDSERVER_H

#include <Arduino.h>

#ifndef NATIVE_BUILD
#include <WebServer.h>
#include <WiFi.h>
#else
#include "network/WebDashboardHandlers.h"
#endif

/**
 * @brief Manages the Wi-Fi softAP and HTTP web server endpoints.
 */
class WebDashboardServer {
public:
    WebDashboardServer();
    void begin();
    void handleClient();
    void stop();

private:
#ifndef NATIVE_BUILD
    WebServer server_;
    bool isRunning_ = false;
    bool routesRegistered_ = false;
    void registerRoutes();
#endif
};

#endif // WEBDASHBOARDSERVER_H
