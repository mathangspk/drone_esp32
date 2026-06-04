#ifndef IIMU_H
#define IIMU_H

/**
 * @brief Abstract interface for the IMU sensor (Inertial Measurement Unit).
 * Supports standard reads and simulated overrides.
 */
class IIMU {
public:
    virtual ~IIMU() = default;

    /**
     * @brief Polls the IMU hardware for fresh accelerometer and gyroscope data.
     */
    virtual void readSensor() = 0;

    /**
     * @brief Gets current calibrated gyroscope rates (deg/s) for roll, pitch, and yaw.
     */
    virtual void getGyroRates(float& rollRate, float& pitchRate, float& yawRate) const = 0;

    /**
     * @brief Gets raw accelerometer-based angles (deg) for roll and pitch.
     */
    virtual void getAccAngles(float& rollAngle, float& pitchAngle) const = 0;

    /**
     * @brief Sets manual override values to simulate custom flight conditions.
     */
    virtual void setOverride(float rollRate, float pitchRate, float yawRate,
                              float rollAngle, float pitchAngle) = 0;

    /**
     * @brief Enables or disables the override mode.
     */
    virtual void setOverrideActive(bool active) = 0;

    /**
     * @brief Checks if override mode is currently active.
     */
    virtual bool isOverrideActive() const = 0;
};

#endif // IIMU_H
