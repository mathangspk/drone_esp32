#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H

#include <Arduino.h>

enum DebugMode {
    MODE_NONE = 0,
    MODE_CONFIG = 1,
    MODE_OPERATOR = 2
};

enum DataType {
    DATA_NONE = 0,
    DATA_ORIENTATION = 1,
    DATA_RECEIVER = 2
};

class SerialDebug {
private:
    DebugMode currentMode;
    DataType currentDataType;
    unsigned long lastPrintTime;
    const unsigned long PRINT_INTERVAL = 20; // 50Hz for operator mode
    const unsigned long CONFIG_INTERVAL = 100; // 10Hz for config mode

    // Buffer for building debug strings
    char buffer[256];

public:
    SerialDebug() : 
        currentMode(MODE_NONE),
        currentDataType(DATA_NONE),
        lastPrintTime(0) {}

    void setMode(DebugMode mode, DataType dataType = DATA_NONE) {
        currentMode = mode;
        currentDataType = dataType;
        // Send acknowledgment
        Serial.printf("MODE:%d,%d\n", mode, dataType);
    }

    void sendOrientationData(float roll, float pitch, float yaw) {
        if (currentMode == MODE_CONFIG && currentDataType == DATA_ORIENTATION) {
            if (millis() - lastPrintTime >= CONFIG_INTERVAL) {
                Serial.printf("ORIENTATION:%.2f,%.2f,%.2f\n", roll, pitch, yaw);
                lastPrintTime = millis();
            }
        }
    }

    void sendReceiverData(uint16_t throttle, uint16_t roll, uint16_t pitch, 
                         uint16_t yaw, uint16_t aux1, uint16_t aux2) {
        if (currentMode == MODE_CONFIG && currentDataType == DATA_RECEIVER) {
            if (millis() - lastPrintTime >= CONFIG_INTERVAL) {
                Serial.printf("RECEIVER:%lu,%u,%u,%u,%u,%u,%u\n",
                    millis(), throttle, roll, pitch, yaw, aux1, aux2);
                lastPrintTime = millis();
            }
        }
    }

    void sendOperatorData(
        float throttle,
        float roll, float pitch, float yaw,
        float rollPID, float pitchPID, float yawPID,
        float rollSetpoint, float pitchSetpoint, float yawSetpoint,
        uint8_t motor1, uint8_t motor2, uint8_t motor3, uint8_t motor4
    ) {
        if (currentMode == MODE_OPERATOR) {
            if (millis() - lastPrintTime >= PRINT_INTERVAL) {
                Serial.printf("OPERATOR:%lu,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%d,%d,%d\n",
                    millis(),
                    throttle,
                    roll, pitch, yaw,
                    rollPID, pitchPID, yawPID,
                    rollSetpoint, pitchSetpoint, yawSetpoint,
                    motor1, motor2, motor3, motor4
                );
                lastPrintTime = millis();
            }
        }
    }

    void handleCommand(const String& cmd) {
        if (cmd.startsWith("CONFIG:")) {
            // Format: CONFIG:mode,datatype
            int mode = cmd.substring(7, 8).toInt();
            int datatype = 0;
            if (cmd.indexOf(',') > 0) {
                datatype = cmd.substring(9).toInt();
            }
            setMode((DebugMode)mode, (DataType)datatype);
        }
        else if (cmd == "SEND_OPERATOR") {
            setMode(MODE_OPERATOR);
        }
        else if (cmd == "STOP_OPERATOR") {
            setMode(MODE_NONE);
        }
    }
};

#endif // SERIAL_DEBUG_H 