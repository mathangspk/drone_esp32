#include "devices/SensorCalibration.h"

// Gyro calibration
void SensorCalibration::startGyroCalibration() {
    Serial.println("=== Bắt đầu Gyro Calibration ===");
    Serial.println("Đặt drone đứng yên trong 10 giây...");
    gyroCal.calibrated = false;
}

void SensorCalibration::addGyroSample(float gx, float gy, float gz) {
    static int sampleCount = 0;
    if (sampleCount < CALIBRATION_SAMPLES) {
        gyroSamplesX[sampleCount] = gx;
        gyroSamplesY[sampleCount] = gy;
        gyroSamplesZ[sampleCount] = gz;
        sampleCount++;
        
        if (sampleCount % 100 == 0) {
            Serial.printf("Gyro samples: %d/%d\n", sampleCount, CALIBRATION_SAMPLES);
        }
    }
}

void SensorCalibration::finishGyroCalibration() {
    // Calculate average bias
    float sumX = 0, sumY = 0, sumZ = 0;
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        sumX += gyroSamplesX[i];
        sumY += gyroSamplesY[i];
        sumZ += gyroSamplesZ[i];
    }
    
    gyroCal.biasX = sumX / CALIBRATION_SAMPLES;
    gyroCal.biasY = sumY / CALIBRATION_SAMPLES;
    gyroCal.biasZ = sumZ / CALIBRATION_SAMPLES;
    gyroCal.calibrated = true;
    
    Serial.printf("Gyro bias: X=%.6f Y=%.6f Z=%.6f\n", 
                  gyroCal.biasX, gyroCal.biasY, gyroCal.biasZ);
    Serial.println("Gyro calibration hoàn tất!");
}

void SensorCalibration::applyGyroCalibration(float& gx, float& gy, float& gz) {
    if (gyroCal.calibrated) {
        gx -= gyroCal.biasX;
        gy -= gyroCal.biasY;
        gz -= gyroCal.biasZ;
    }
}

// Accelerometer calibration (simplified 6-point)
void SensorCalibration::startAccelCalibration() {
    Serial.println("=== Bắt đầu Accel Calibration ===");
    Serial.println("Đặt drone nằm phẳng (face up)...");
    accelCal.calibrated = false;
}

void SensorCalibration::addAccelSample(float ax, float ay, float az) {
    // Simplified: just use face-up position for offset calibration
    static int sampleCount = 0;
    if (sampleCount < CALIBRATION_SAMPLES) {
        // Face up: Z should be ~9.81, X and Y should be ~0
        accelCal.offsetX = ax;
        accelCal.offsetY = ay;
        accelCal.offsetZ = az - 9.81f; // Assuming 1g = 9.81 m/s²
        sampleCount++;
        
        if (sampleCount % 100 == 0) {
            Serial.printf("Accel samples: %d/%d\n", sampleCount, CALIBRATION_SAMPLES);
        }
    }
}

void SensorCalibration::finishAccelCalibration() {
    accelCal.calibrated = true;
    Serial.printf("Accel offset: X=%.3f Y=%.3f Z=%.3f\n", 
                  accelCal.offsetX, accelCal.offsetY, accelCal.offsetZ);
    Serial.println("Accel calibration hoàn tất!");
}

void SensorCalibration::applyAccelCalibration(float& ax, float& ay, float& az) {
    if (accelCal.calibrated) {
        ax -= accelCal.offsetX;
        ay -= accelCal.offsetY;
        az -= accelCal.offsetZ;
    }
}

// Magnetometer calibration (simplified)
void SensorCalibration::startMagCalibration() {
    Serial.println("=== Bắt đầu Mag Calibration ===");
    Serial.println("Xoay drone trong tất cả hướng...");
    magCal.calibrated = false;
}

void SensorCalibration::addMagSample(float mx, float my, float mz) {
    // Simplified: just track min/max for basic calibration
    static float minX = mx, maxX = mx;
    static float minY = my, maxY = my;
    static float minZ = mz, maxZ = mz;
    static int sampleCount = 0;
    
    if (sampleCount < CALIBRATION_SAMPLES) {
        minX = min(minX, mx);
        maxX = max(maxX, mx);
        minY = min(minY, my);
        maxY = max(maxY, my);
        minZ = min(minZ, mz);
        maxZ = max(maxZ, mz);
        sampleCount++;
        
        if (sampleCount % 100 == 0) {
            Serial.printf("Mag samples: %d/%d\n", sampleCount, CALIBRATION_SAMPLES);
        }
    }
}

void SensorCalibration::finishMagCalibration() {
    // Calculate offset and scale
    static float minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
    
    magCal.offsetX = (maxX + minX) / 2.0f;
    magCal.offsetY = (maxY + minY) / 2.0f;
    magCal.offsetZ = (maxZ + minZ) / 2.0f;
    
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    float rangeZ = maxZ - minZ;
    
    magCal.scaleX = rangeX > 0 ? 1.0f / rangeX : 1.0f;
    magCal.scaleY = rangeY > 0 ? 1.0f / rangeY : 1.0f;
    magCal.scaleZ = rangeZ > 0 ? 1.0f / rangeZ : 1.0f;
    
    magCal.calibrated = true;
    
    Serial.printf("Mag offset: X=%.3f Y=%.3f Z=%.3f\n", 
                  magCal.offsetX, magCal.offsetY, magCal.offsetZ);
    Serial.printf("Mag scale: X=%.3f Y=%.3f Z=%.3f\n", 
                  magCal.scaleX, magCal.scaleY, magCal.scaleZ);
    Serial.println("Mag calibration hoàn tất!");
}

void SensorCalibration::applyMagCalibration(float& mx, float& my, float& mz) {
    if (magCal.calibrated) {
        mx = (mx - magCal.offsetX) * magCal.scaleX;
        my = (my - magCal.offsetY) * magCal.scaleY;
        mz = (mz - magCal.offsetZ) * magCal.scaleZ;
    }
}

// Apply all calibrations and filtering
void SensorCalibration::processGyro(float& gx, float& gy, float& gz) {
    // Apply calibration
    applyGyroCalibration(gx, gy, gz);
    
    // Apply low-pass filter
    filteredGyroX = lowPassFilter(gx, filteredGyroX, gyroAlpha);
    filteredGyroY = lowPassFilter(gy, filteredGyroY, gyroAlpha);
    filteredGyroZ = lowPassFilter(gz, filteredGyroZ, gyroAlpha);
    
    // Return filtered values
    gx = filteredGyroX;
    gy = filteredGyroY;
    gz = filteredGyroZ;
}

void SensorCalibration::processAccel(float& ax, float& ay, float& az) {
    // Apply calibration
    applyAccelCalibration(ax, ay, az);
    
    // Apply low-pass filter
    filteredAccelX = lowPassFilter(ax, filteredAccelX, accelAlpha);
    filteredAccelY = lowPassFilter(ay, filteredAccelY, accelAlpha);
    filteredAccelZ = lowPassFilter(az, filteredAccelZ, accelAlpha);
    
    // Return filtered values
    ax = filteredAccelX;
    ay = filteredAccelY;
    az = filteredAccelZ;
}

void SensorCalibration::processMag(float& mx, float& my, float& mz) {
    // Apply calibration
    applyMagCalibration(mx, my, mz);
    
    // Apply low-pass filter
    filteredMagX = lowPassFilter(mx, filteredMagX, magAlpha);
    filteredMagY = lowPassFilter(my, filteredMagY, magAlpha);
    filteredMagZ = lowPassFilter(mz, filteredMagZ, magAlpha);
    
    // Return filtered values
    mx = filteredMagX;
    my = filteredMagY;
    mz = filteredMagZ;
}

// Reset all calibrations
void SensorCalibration::resetAll() {
    gyroCal.calibrated = false;
    accelCal.calibrated = false;
    magCal.calibrated = false;
    
    // Reset filtered values
    filteredGyroX = filteredGyroY = filteredGyroZ = 0.0f;
    filteredAccelX = filteredAccelY = filteredAccelZ = 0.0f;
    filteredMagX = filteredMagY = filteredMagZ = 0.0f;
    
    Serial.println("Tất cả calibrations đã reset!");
}

// Print calibration data
void SensorCalibration::printCalibrationData() {
    Serial.println("=== Calibration Data ===");
    
    Serial.printf("Gyro calibrated: %s\n", gyroCal.calibrated ? "YES" : "NO");
    if (gyroCal.calibrated) {
        Serial.printf("Gyro bias: X=%.6f Y=%.6f Z=%.6f\n", 
                      gyroCal.biasX, gyroCal.biasY, gyroCal.biasZ);
    }
    
    Serial.printf("Accel calibrated: %s\n", accelCal.calibrated ? "YES" : "NO");
    if (accelCal.calibrated) {
        Serial.printf("Accel offset: X=%.3f Y=%.3f Z=%.3f\n", 
                      accelCal.offsetX, accelCal.offsetY, accelCal.offsetZ);
    }
    
    Serial.printf("Mag calibrated: %s\n", magCal.calibrated ? "YES" : "NO");
    if (magCal.calibrated) {
        Serial.printf("Mag offset: X=%.3f Y=%.3f Z=%.3f\n", 
                      magCal.offsetX, magCal.offsetY, magCal.offsetZ);
        Serial.printf("Mag scale: X=%.3f Y=%.3f Z=%.3f\n", 
                      magCal.scaleX, magCal.scaleY, magCal.scaleZ);
    }
} 