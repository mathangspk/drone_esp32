#ifndef KALMANFILTER_H
#define KALMANFILTER_H

/**
 * @brief Platform-independent 1D Kalman filter.
 * Estimates quadcopter roll/pitch angles by fusing gyroscope rates and accelerometer angles.
 */
class KalmanFilter {
public:
    /**
     * @brief Construct a new KalmanFilter object.
     */
    KalmanFilter(float initialState = 0.0f, float initialUncertainty = 4.0f);

    /**
     * @brief Performs standard 1D Kalman filter fusion.
     * @param rate Gyroscope angular rate (deg/s).
     * @param measurement Accelerometer-calculated angle (deg).
     * @param dt Sampling time delta (s).
     */
    void update(float rate, float measurement, float dt);

    /**
     * @brief Resets filter states.
     */
    void reset(float state = 0.0f, float uncertainty = 4.0f);

    // Getters
    float getState() const { return state_; }
    float getUncertainty() const { return uncertainty_; }

private:
    float state_;
    float uncertainty_;
};

#endif // KALMANFILTER_H
