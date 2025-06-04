#pragma once

class MadgwickEstimator {
public:
    MadgwickEstimator();

    void begin();
    void update(float ax, float ay, float az, float gx, float gy, float gz, float dt);

    float getRoll() const;
    float getPitch() const;
    float getYaw() const;      // Lấy yaw bằng gyroZ tích phân
    void calibrateYaw();       // Đặt góc yaw hiện tại làm 0

private:
    void computeAngles();

    // Quaternion
    float q0, q1, q2, q3;

    // Euler angles (rad)
    float roll, pitch, yaw;
    float yawOffset;

    // Yaw tính bằng gyroZ tích phân
    float yawGyro;

    bool anglesComputed;
};