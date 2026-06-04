#include "network/WebDashboardHandlers.h"
#include "core/FlightController.h"

FlightLogEntry WebDashboardHandlers::logBuffer_[WebDashboardHandlers::MAX_LOGS];
int WebDashboardHandlers::logIndex_ = 0;
int WebDashboardHandlers::logCount_ = 0;

#ifndef NATIVE_BUILD
SemaphoreHandle_t WebDashboardHandlers::logMutex_ = nullptr;
#endif

void WebDashboardHandlers::handleGetLog(WebServer& server) {
    String csv = "time_ms,roll_sp,roll_act,pitch_sp,pitch_act,yaw_sp,yaw_act,throttle,m1,m2,m3,m4,voltage\n";
#ifndef NATIVE_BUILD
    if (xSemaphoreTake(logMutex_, pdMS_TO_TICKS(10)) != pdTRUE) {
        server.send(503, "text/plain", "Log busy");
        return;
    }
#endif
    int startIdx = logCount_ < MAX_LOGS ? 0 : logIndex_;
    int count = logCount_;
    for (int i = 0; i < count; i++) {
        int idx = (startIdx + i) % MAX_LOGS;
        const FlightLogEntry& e = logBuffer_[idx];
        char line[192];
        snprintf(line, sizeof(line),
                 "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%d,%d,%.2f\n",
                 (unsigned long)e.timeMs,
                 e.rollSp, e.rollAct, e.pitchSp, e.pitchAct,
                 e.yawSp, e.yawAct, e.throttle,
                 e.m1, e.m2, e.m3, e.m4, e.voltage);
        csv += line;
    }
#ifndef NATIVE_BUILD
    xSemaphoreGive(logMutex_);
#endif
    server.send(200, "text/plain", csv);
}

void WebDashboardHandlers::logFlightData(float rSp, float rAct, float pSp, float pAct,
                                         float ySp, float yAct, int16_t throttle,
                                         int16_t m1, int16_t m2, int16_t m3, int16_t m4,
                                         float voltage) {
    if (!ppm_ || ppm_->getChannel(FlightController::ARM_CHANNEL) <= FlightController::ARM_THRESHOLD) return;
#ifndef NATIVE_BUILD
    if (xSemaphoreTake(logMutex_, 0) != pdTRUE) return; // skip if log is being read
#endif
    logBuffer_[logIndex_] = {
        static_cast<uint32_t>(millis()),
        rSp, rAct, pSp, pAct, ySp, yAct,
        throttle, m1, m2, m3, m4, voltage
    };
    logIndex_ = (logIndex_ + 1) % MAX_LOGS;
    if (logCount_ < MAX_LOGS) logCount_++;
#ifndef NATIVE_BUILD
    xSemaphoreGive(logMutex_);
#endif
}

void WebDashboardHandlers::clearFlightLog() {
    logIndex_ = 0; logCount_ = 0;
}
