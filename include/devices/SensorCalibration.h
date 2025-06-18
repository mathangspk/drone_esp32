#ifndef SENSOR_CALIBRATION_H
#define SENSOR_CALIBRATION_H

#include <Arduino.h>

// Gyro bias calibration
struct GyroCalibration {
    float biasX = 0.0f;
    float biasY = 0.0f;
    float biasZ = 0.0f;
    bool calibrated = false;
};

// Accelerometer calibration (6-point)
struct AccelCalibration {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;
    bool calibrated = false;
};

// Magnetometer calibration (12-point)
struct MagCalibration {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;
    bool calibrated = false;
};

class SensorCalibration {
private:
    GyroCalibration gyroCal;
    AccelCalibration accelCal;
    MagCalibration magCal;
    
    // Calibration samples
    static const int CALIBRATION_SAMPLES = 1000;
    float gyroSamplesX[CALIBRATION_SAMPLES];
    float gyroSamplesY[CALIBRATION_SAMPLES];
    float gyroSamplesZ[CALIBRATION_SAMPLES];
    
    // Low-pass filter parameters
    float gyroAlpha = 0.1f;
    float accelAlpha = 0.1f;
    float magAlpha = 0.1f;
    
    // Filtered values
    float filteredGyroX = 0.0f, filteredGyroY = 0.0f, filteredGyroZ = 0.0f;
    float filteredAccelX = 0.0f, filteredAccelY = 0.0f, filteredAccelZ = 0.0f;
    float filteredMagX = 0.0f, filteredMagY = 0.0f, filteredMagZ = 0.0f;

public:
    SensorCalibration() {}
    
    // Low-pass filter
    float lowPassFilter(float input, float output, float alpha) {
        return alpha * input + (1.0f - alpha) * output;
    }
    
    // Gyro calibration
    void startGyroCalibration();
    void addGyroSample(float gx, float gy, float gz);
    void finishGyroCalibration();
    void applyGyroCalibration(float& gx, float& gy, float& gz);
    
    // Accelerometer calibration
    void startAccelCalibration();
    void addAccelSample(float ax, float ay, float az);
    void finishAccelCalibration();
    void applyAccelCalibration(float& ax, float& ay, float& az);
    
    // Magnetometer calibration
    void startMagCalibration();
    void addMagSample(float mx, float my, float mz);
    void finishMagCalibration();
    void applyMagCalibration(float& mx, float& my, float& mz);
    
    // Apply all calibrations and filtering
    void processGyro(float& gx, float& gy, float& gz);
    void processAccel(float& ax, float& ay, float& az);
    void processMag(float& mx, float& my, float& mz);
    
    // Reset all calibrations
    void resetAll();
    
    // Get calibration status
    bool isGyroCalibrated() { return gyroCal.calibrated; }
    bool isAccelCalibrated() { return accelCal.calibrated; }
    bool isMagCalibrated() { return magCal.calibrated; }
    
    // Set filter parameters
    void setGyroFilter(float alpha) { gyroAlpha = alpha; }
    void setAccelFilter(float alpha) { accelAlpha = alpha; }
    void setMagFilter(float alpha) { magAlpha = alpha; }
    
    // Print calibration data
    void printCalibrationData();
};

#endif // SENSOR_CALIBRATION_H 