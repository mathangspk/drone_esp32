#include "network/WebDashboardHandlers.h"
#include "core/FlightController.h"

FlightLogEntry WebDashboardHandlers::logBuffer_[WebDashboardHandlers::MAX_LOGS];
int WebDashboardHandlers::logIndex_ = 0;
int WebDashboardHandlers::logCount_ = 0;

#ifndef NATIVE_BUILD
SemaphoreHandle_t WebDashboardHandlers::logMutex_ = nullptr;
#endif

void WebDashboardHandlers::handleGetLog(WebServer& server) {
    String csv = "time_ms,roll_sp,roll_act,pitch_sp,pitch_act,throttle\n";
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
        char line[128];
        snprintf(line, sizeof(line), "%d,%.2f,%.2f,%.2f,%.2f,%d\n",
                 logBuffer_[idx].timeMs, logBuffer_[idx].rollSp, logBuffer_[idx].rollAct,
                 logBuffer_[idx].pitchSp, logBuffer_[idx].pitchAct, logBuffer_[idx].throttle);
        csv += line;
    }
#ifndef NATIVE_BUILD
    xSemaphoreGive(logMutex_);
#endif
    server.send(200, "text/plain", csv);
}

void WebDashboardHandlers::logFlightData(float rSp, float rAct, float pSp, float pAct, int16_t throttle) {
    if (!ppm_ || ppm_->getChannel(FlightController::ARM_CHANNEL) <= FlightController::ARM_THRESHOLD) return;
#ifndef NATIVE_BUILD
    if (xSemaphoreTake(logMutex_, 0) != pdTRUE) return; // skip if log is being read
#endif
    logBuffer_[logIndex_] = { static_cast<uint32_t>(millis()), rSp, rAct, pSp, pAct, throttle };
    logIndex_ = (logIndex_ + 1) % MAX_LOGS;
    if (logCount_ < MAX_LOGS) logCount_++;
#ifndef NATIVE_BUILD
    xSemaphoreGive(logMutex_);
#endif
}

void WebDashboardHandlers::clearFlightLog() {
    logIndex_ = 0; logCount_ = 0;
}
