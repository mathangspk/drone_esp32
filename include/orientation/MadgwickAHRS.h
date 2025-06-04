// MadgwickAHRS.h
#pragma once

class MadgwickAHRS {
public:
    MadgwickAHRS(float beta = 0.1f);
    void update(float gx, float gy, float gz, float ax, float ay, float az, float dt);
    float getRoll() const;
    float getPitch() const;
    float getYaw() const;

private:
    float q0, q1, q2, q3; // Quaternion
    float beta;           // Algorithm gain
};
