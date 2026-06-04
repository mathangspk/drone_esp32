#ifndef IBATTERY_H
#define IBATTERY_H

/**
 * @brief Abstract interface for battery telemetry.
 * Monitors battery voltage and safety limits, with override support.
 */
class IBattery {
public:
    virtual ~IBattery() = default;

    static constexpr float LOW_VOLTAGE_THRESHOLD = 9.0f; // 3.0V/cell critical on 3S LiPo

    /**
     * @brief Reads the current battery voltage in Volts.
     */
    virtual float readVoltage() const = 0;

    /**
     * @brief Checks if battery voltage is below the safe discharge threshold (typically 9.0V).
     */
    virtual bool isLow() const = 0;

    /**
     * @brief Overrides the battery reading with a simulated voltage.
     */
    virtual void setOverride(float voltage) = 0;

    /**
     * @brief Enables or disables the voltage override mode.
     */
    virtual void setOverrideActive(bool active) = 0;

    /**
     * @brief Checks if override mode is currently active.
     */
    virtual bool isOverrideActive() const = 0;
};

#endif // IBATTERY_H
