#ifndef PWMESP32MOTORS_H
#define PWMESP32MOTORS_H

#include "interfaces/IMotors.h"

/**
 * @brief ESP32 Brushless Motor ESC driver using LEDC hardware PWM.
 */
class PWMESP32Motors : public IMotors {
public:
    PWMESP32Motors(int pinM1, int pinM2, int pinM3, int pinM4);

    void init();
    void writeMotors(int m1, int m2, int m3, int m4) override;

    // Simulation/Override functionality
    void setOverride(int motorIdx, int value, bool active) override;
    int getMotorOutput(int motorIdx) const override;
    bool isMotorOverridden(int motorIdx) const override;

private:
    static constexpr int LEDC_FREQ_HZ  = 250;
    static constexpr int LEDC_BITS     = 12;
    static constexpr int PWM_PERIOD_US = 1000000 / LEDC_FREQ_HZ; // 4000
    static constexpr int LEDC_MAX_DUTY = 1 << LEDC_BITS;          // 4096

    static constexpr int usToDuty(int us) {
        return (us * LEDC_MAX_DUTY) / PWM_PERIOD_US;
    }

    int pins_[4];
    int outputs_[4] = {1000, 1000, 1000, 1000};

    // Override states
    bool oActive_[4] = {false, false, false, false};
    int oVal_[4] = {1000, 1000, 1000, 1000};
};

#endif // PWMESP32MOTORS_H
