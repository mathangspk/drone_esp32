#ifndef ISTATUSINDICATOR_H
#define ISTATUSINDICATOR_H

/**
 * @brief Abstract interface for status indicators (LED, buzzer, etc.)
 */
class IStatusIndicator {
public:
    virtual ~IStatusIndicator() = default;
    
    /**
     * @brief Initialize the hardware indicator.
     */
    virtual void init() = 0;
    
    /**
     * @brief Update the low battery state.
     */
    virtual void setLowBattery(bool isLow) = 0;
    
    /**
     * @brief Update the armed/ready state.
     */
    virtual void setArmed(bool isArmed) = 0;
    
    /**
     * @brief Run the non-blocking state machine to update physical outputs (blink, beep, etc.)
     */
    virtual void update() = 0;
};

#endif // ISTATUSINDICATOR_H
