#ifndef WEBSOCKET_DEBUG_H
#define WEBSOCKET_DEBUG_H

#include <Arduino.h>

class WebSocketDebug {
private:
    unsigned long lastSendTime;
    const unsigned long SEND_INTERVAL = 50; // 20Hz for real-time plotting
    bool enabled;

public:
    WebSocketDebug() : 
        lastSendTime(0),
        enabled(false) {}

    void begin() {
        enabled = true;
        Serial.println("WebSocketDebug: Ready for HTTP polling");
    }

    void handle() {
        // No WebSocket handling needed for HTTP polling
    }

    void sendDebugData(
        float roll, float pitch, float yaw,
        float rollPID, float pitchPID, float yawPID,
        float rollSetpoint, float pitchSetpoint, float yawSetpoint,
        uint8_t motor1, uint8_t motor2, uint8_t motor3, uint8_t motor4,
        float voltage, float current,
        float ax, float ay, float az,
        float gx, float gy, float gz
    ) {
        if (!enabled) return;
        
        unsigned long currentTime = millis();
        if (currentTime - lastSendTime >= SEND_INTERVAL) {
            lastSendTime = currentTime;
            
            // Send data via Serial for debugging
            Serial.printf("DEBUG_DATA:%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                currentTime,
                roll, pitch, yaw,
                rollPID, pitchPID, yawPID,
                rollSetpoint, pitchSetpoint, yawSetpoint,
                motor1, motor2, motor3, motor4,
                voltage, current,
                ax, ay, az,
                gx, gy, gz
            );
        }
    }

    bool isEnabled() const { return enabled; }
};

#endif // WEBSOCKET_DEBUG_H 