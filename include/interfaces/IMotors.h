#ifndef IMOTORS_H
#define IMOTORS_H

/**
 * @brief Abstract interface for controlling ESC motor outputs.
 * Directs motor speeds and tracks final outputs, with override capabilities.
 */
class IMotors {
public:
    virtual ~IMotors() = default;

    /**
     * @brief Writes output speed values to the 4 ESC motors (typical range: 1000 to 2000).
     */
    virtual void writeMotors(int m1, int m2, int m3, int m4) = 0;

    /**
     * @brief Simulates physical hardware conditions by overriding a motor speed output.
     */
    virtual void setOverride(int motorIdx, int value, bool active) = 0;

    /**
     * @brief Gets what speed is currently being output to the physical or simulated motor.
     */
    virtual int getMotorOutput(int motorIdx) const = 0;

    /**
     * @brief Checks if a specific motor's output is currently overridden.
     */
    virtual bool isMotorOverridden(int motorIdx) const = 0;
};

#endif // IMOTORS_H
