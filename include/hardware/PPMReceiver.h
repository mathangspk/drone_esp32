#ifndef PPMRECEIVER_H
#define PPMRECEIVER_H

#include "interfaces/IPPM.h"

#ifndef NATIVE_BUILD
#include <PPMReader.h>
#endif

/**
 * @brief ESP32 PPM RC receiver driver. Wraps interrupt-driven PPMReader library.
 */
class PPMReceiver : public IPPM {
public:
    PPMReceiver(int interruptPin, int channelAmount);

    void readChannels() override;
    int getChannel(int channelIdx) const override;
    bool isSignalLost() const override;

    // Simulation/Override functionality
    void setOverride(int channelIdx, int value) override;
    void setSignalLostOverride(bool lost) override { overrideSignalLost_ = lost; }
    void setOverrideActive(bool active) override { overrideActive_ = active; }
    bool isOverrideActive() const override { return overrideActive_; }

private:
#ifndef NATIVE_BUILD
    PPMReader ppm_;
#endif
    int channelAmount_;
    int channels_[6] = {1500, 1500, 1000, 1500, 1500, 1500};
    unsigned long lastPPMUpdateTime_ = 0;
    bool ppmSignalLost_ = true;

    // Simulation states
    bool overrideActive_ = false;
    bool overrideSignalLost_ = false;
    int oChannels_[6] = {1500, 1500, 1000, 1500, 1500, 1500};
};

#endif // PPMRECEIVER_H
