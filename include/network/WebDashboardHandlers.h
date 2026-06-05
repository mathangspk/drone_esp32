#ifndef WEBDASHBOARDHANDLERS_H
#define WEBDASHBOARDHANDLERS_H

#ifndef NATIVE_BUILD
#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#else
#include <string>
#include <cstdint>
using String = std::string;
class WebServer {
public:
    void send(int code, const char* content_type, const String& content) {}
    void send(int code, const char* content_type, const char* content) {}
    String arg(const char* name) { return ""; }
};
#endif

#include "interfaces/IPPM.h"
#include "interfaces/IMotors.h"
#include "interfaces/IBattery.h"
#include "interfaces/IIMU.h"

struct FlightLogEntry {
    uint32_t timeMs;
    float rollSp,  rollAct;
    float pitchSp, pitchAct;
    float yawSp,   yawAct;
    int16_t throttle;
    int16_t m1, m2, m3, m4;
    float voltage;
};

/**
 * @brief Handles API request callbacks from the Web Dashboard and manages the RAM Blackbox buffer.
 */
class WebDashboardHandlers {
public:
    static void init(IPPM& ppm, IMotors& motors, IBattery& battery, IIMU& imu);

    static void handleRoot(WebServer& server);
    static void handleGetPID(WebServer& server);
    static void handleSetPID(WebServer& server);
    static void handleGetReceiver(WebServer& server);
    static void handleSetReceiver(WebServer& server);
    static void handleMotorTest(WebServer& server);
    static void handleCalibrateESC(WebServer& server);
    static void handleGetIMU(WebServer& server);
    static void handleGetLog(WebServer& server);

    static void logFlightData(float rSp, float rAct, float pSp, float pAct,
                              float ySp, float yAct, int16_t throttle,
                              int16_t m1, int16_t m2, int16_t m3, int16_t m4,
                              float voltage);
    static void clearFlightLog();

private:
    static IPPM* ppm_;
    static IMotors* motors_;
    static IBattery* battery_;
    static IIMU* imu_;

    static const int MAX_LOGS = 500; // 500 entries @ 50Hz = 10 seconds of log
    static FlightLogEntry logBuffer_[MAX_LOGS];
    static int logIndex_;
    static int logCount_;

#ifndef NATIVE_BUILD
    static SemaphoreHandle_t logMutex_;
#endif
};

#endif // WEBDASHBOARDHANDLERS_H
