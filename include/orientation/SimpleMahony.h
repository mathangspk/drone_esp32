#ifndef SIMPLE_MAHONY_H
#define SIMPLE_MAHONY_H

#include <Arduino.h>
#include <math.h>

class SimpleMahony {
private:
    // Quaternion components
    float q0, q1, q2, q3;

    // Algorithm gains
    float twoKp;         // 2 * proportional gain
    float twoKi;         // 2 * integral gain
    float twoKm;         // 2 * magnetometer gain

    // Integral error terms
    float integralFBx, integralFBy, integralFBz;

    // Sampling frequency
    float sampleFreq;    // Sample frequency in Hz
    float invSampleFreq; // 1 / sample frequency

    // Euler angles
    float roll, pitch, yaw;

    // Private method to compute angles
    void computeAngles();

public:
    // Constructor
    SimpleMahony(float sampleFrequency = 100.0f);

    // Main update functions
    void update(float gx, float gy, float gz, float ax, float ay, float az);
    void update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

    // Getters for angles in radians
    float getRoll() { return roll; }
    float getPitch() { return pitch; }
    float getYaw() { return yaw; }

    // Getters for angles in degrees
    float getRollDegrees() { return roll * RAD_TO_DEG; }
    float getPitchDegrees() { return pitch * RAD_TO_DEG; }
    float getYawDegrees() { return yaw * RAD_TO_DEG; }

    // Method to set algorithm gains
    void setGains(float proportional, float integral, float magnetic = 1.0f);

    // Reset method
    void reset();
};

#endif // SIMPLE_MAHONY_H