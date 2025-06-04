#ifndef I_ORIENTATION_ESTIMATOR_H
#define I_ORIENTATION_ESTIMATOR_H

class IOrientationEstimator {
public:
    virtual void begin() = 0;
    virtual void update(float ax, float ay, float az, float gx, float gy, float gz, float dt) = 0;
    virtual float getRoll() const = 0;
    virtual float getPitch() const = 0;
    virtual float getYaw() const = 0;
    virtual void calibrate() = 0;
    virtual ~IOrientationEstimator() {}
};

#endif
