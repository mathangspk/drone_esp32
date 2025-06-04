#include "orientation/SimpleMahony.h"

SimpleMahony::SimpleMahony(float sampleFrequency) {
    // Initialize quaternion
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    
    // Initialize gains
    twoKp = 2.0f * 5.0f;  // 2 * proportional gain
    twoKi = 2.0f * 0.0f;  // 2 * integral gain
    twoKm = 2.0f * 1.0f;  // 2 * magnetic gain
    
    // Initialize integral error terms
    integralFBx = 0.0f;
    integralFBy = 0.0f;
    integralFBz = 0.0f;
    
    // Initialize timing
    sampleFreq = sampleFrequency;
    invSampleFreq = 1.0f / sampleFreq;

    // Initialize angles
    roll = pitch = yaw = 0.0f;
}

void SimpleMahony::update(float gx, float gy, float gz, float ax, float ay, float az) {
    float recipNorm;
    float halfvx, halfvy, halfvz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    // Only compute if accelerometer measurement valid
    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        // Normalize accelerometer measurement
        recipNorm = 1.0f / sqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Estimated direction of gravity
        halfvx = q1 * q3 - q0 * q2;
        halfvy = q0 * q1 + q2 * q3;
        halfvz = q0 * q0 - 0.5f + q3 * q3;

        // Error is cross product between estimated and measured direction of gravity
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);

        // Apply integral feedback if enabled
        if(twoKi > 0.0f) {
            integralFBx += twoKi * halfex * invSampleFreq;
            integralFBy += twoKi * halfey * invSampleFreq;
            integralFBz += twoKi * halfez * invSampleFreq;
            gx += integralFBx;
            gy += integralFBy;
            gz += integralFBz;
        } else {
            integralFBx = 0.0f;
            integralFBy = 0.0f;
            integralFBz = 0.0f;
        }

        // Apply proportional feedback
        gx += twoKp * halfex;
        gy += twoKp * halfey;
        gz += twoKp * halfez;
    }

    // Integrate rate of change of quaternion
    float deltaT = invSampleFreq / 2.0f;
    gx *= deltaT;   // pre-multiply common factors
    gy *= deltaT;
    gz *= deltaT;
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    // Normalise quaternion
    recipNorm = 1.0f / sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    // Compute angles
    computeAngles();
}

void SimpleMahony::update(float gx, float gy, float gz, float ax, float ay, float az, 
                         float mx, float my, float mz) {
    float recipNorm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;  
    float hx, hy, bx, bz;
    float halfvx, halfvy, halfvz;
    float halfwx, halfwy, halfwz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    // Convert gyroscope degrees/sec to radians/sec
    gx *= 0.0174533f;
    gy *= 0.0174533f;
    gz *= 0.0174533f;

    // Compute feedback only if accelerometer measurement valid
    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        // Normalise accelerometer measurement
        recipNorm = 1.0f / sqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;     

        // Normalise magnetometer measurement
        recipNorm = 1.0f / sqrt(mx * mx + my * my + mz * mz);
        mx *= recipNorm;
        my *= recipNorm;
        mz *= recipNorm;   

        // Auxiliary variables to avoid repeated arithmetic
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;   

        // Reference direction of Earth's magnetic field
        hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
        hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
        bx = sqrt(hx * hx + hy * hy);
        bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

        // Estimated direction of gravity and magnetic field
        halfvx = q1q3 - q0q2;
        halfvy = q0q1 + q2q3;
        halfvz = q0q0 - 0.5f + q3q3;
        halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
        halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
        halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);  

        // Error is sum of cross product between estimated direction and measured direction of field vectors
        halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
        halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
        halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

        // Compute and apply integral feedback if enabled
        if(twoKi > 0.0f) {
            // integral error scaled by Ki
            integralFBx += twoKi * halfex * invSampleFreq;
            integralFBy += twoKi * halfey * invSampleFreq;
            integralFBz += twoKi * halfez * invSampleFreq;
            gx += integralFBx;	// apply integral feedback
            gy += integralFBy;
            gz += integralFBz;
        } else {
            integralFBx = 0.0f;	// prevent integral windup
            integralFBy = 0.0f;
            integralFBz = 0.0f;
        }

        // Apply proportional feedback
        gx += twoKp * halfex + twoKm * (my * halfwz - mz * halfwy);
        gy += twoKp * halfey + twoKm * (mz * halfwx - mx * halfwz);
        gz += twoKp * halfez + twoKm * (mx * halfwy - my * halfwx);
    }
    
    // Integrate rate of change of quaternion
    float deltaT = invSampleFreq / 2.0f;
    gx *= deltaT;		// pre-multiply common factors
    gy *= deltaT;
    gz *= deltaT;
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);
    
    // Normalise quaternion
    recipNorm = 1.0f / sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    // Compute angles
    computeAngles();
}

void SimpleMahony::computeAngles() {
    roll = atan2f(q0*q1 + q2*q3, 0.5f - q1*q1 - q2*q2);
    pitch = asinf(-2.0f * (q1*q3 - q0*q2));
    yaw = atan2f(q1*q2 + q0*q3, 0.5f - q2*q2 - q3*q3);
}

void SimpleMahony::setGains(float proportional, float integral, float magnetic) {
    twoKp = 2.0f * proportional;
    twoKi = 2.0f * integral;
    twoKm = 2.0f * magnetic;
}

void SimpleMahony::reset() {
    // Reset quaternion
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    
    // Reset integral terms
    integralFBx = 0.0f;
    integralFBy = 0.0f;
    integralFBz = 0.0f;

    // Reset angles
    roll = pitch = yaw = 0.0f;
}