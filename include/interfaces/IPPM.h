#ifndef IPPM_H
#define IPPM_H

/**
 * @brief Abstract interface for the PPM RC receiver.
 * Manages RC channel inputs and signal loss states, with override support.
 */
class IPPM {
public:
    virtual ~IPPM() = default;

    /**
     * @brief Polls fresh channel inputs from the receiver.
     */
    virtual void readChannels() = 0;

    /**
     * @brief Gets current pulse width (1000 - 2000 us) for a specific channel (0-indexed).
     */
    virtual int getChannel(int channelIdx) const = 0;

    /**
     * @brief Checks if the PPM signal is lost.
     */
    virtual bool isSignalLost() const = 0;

    /**
     * @brief Manually overrides the value of a channel for simulation.
     */
    virtual void setOverride(int channelIdx, int value) = 0;

    /**
     * @brief Simulates receiver signal loss.
     */
    virtual void setSignalLostOverride(bool lost) = 0;

    /**
     * @brief Enables or disables the input overrides.
     */
    virtual void setOverrideActive(bool active) = 0;

    /**
     * @brief Checks if input override is active.
     */
    virtual bool isOverrideActive() const = 0;
};

#endif // IPPM_H
