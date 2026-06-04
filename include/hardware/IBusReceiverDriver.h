#ifndef IBUSRECEIVERDRIVER_H
#define IBUSRECEIVERDRIVER_H

#include "interfaces/IPPM.h"
#include <Arduino.h>

/**
 * @brief i-BUS RC receiver driver implementing the IPPM interface.
 * Reads digital RC channel inputs over hardware serial interface.
 */
class IBusReceiverDriver : public IPPM {
public:
    IBusReceiverDriver(HardwareSerial* serial);
    void begin();

    void readChannels() override;
    int getChannel(int channelIdx) const override;
    bool isSignalLost() const override;

    void setOverride(int channelIdx, int value) override;
    void setSignalLostOverride(bool lost) override { oSignalLost_ = lost; }
    void setOverrideActive(bool active) override { oActive_ = active; }
    bool isOverrideActive() const override { return oActive_; }

private:
    HardwareSerial* serial_;
    uint8_t buffer_[32];
    uint8_t bufIndex_ = 0;
    uint16_t channels_[14];
    unsigned long lastReadTime_ = 0;
    bool signalLost_ = true;

    // Simulation overrides
    bool oActive_ = false;
    bool oSignalLost_ = false;
    int oChannels_[6] = {1500, 1500, 1000, 1500, 1500, 1500};

    bool readByte(uint8_t* byte);
    bool verifyChecksum();
    void updateChannels();
};

#endif // IBUSRECEIVERDRIVER_H
