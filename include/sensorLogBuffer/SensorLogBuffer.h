#pragma once

#include <Arduino.h>

#define LOG_BUFFER_SIZE 256

struct SensorLogSample {
    unsigned long t;
    float roll, pitch, yaw;
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
};

class SensorLogBuffer {
public:
    SensorLogSample buffer[LOG_BUFFER_SIZE];
    volatile int head = 0;
    volatile int tail = 0;

    void push(const SensorLogSample& s) {
        buffer[head] = s;
        head = (head + 1) % LOG_BUFFER_SIZE;
        // Nếu đầy thì ghi đè mẫu cũ nhất
        if (head == tail) tail = (tail + 1) % LOG_BUFFER_SIZE;
    }

    bool pop(SensorLogSample& s) {
        if (tail == head) return false;
        s = buffer[tail];
        tail = (tail + 1) % LOG_BUFFER_SIZE;
        return true;
    }

    bool isEmpty() const { return head == tail; }
};