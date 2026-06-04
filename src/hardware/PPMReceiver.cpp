#include "hardware/PPMReceiver.h"
#include <cmath>

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

PPMReceiver::PPMReceiver(int interruptPin, int channelAmount)
    : 
#ifndef NATIVE_BUILD
      ppm_(interruptPin, channelAmount),
#endif
      channelAmount_(channelAmount) {}

void PPMReceiver::readChannels() {
    if (overrideActive_) return;

#ifndef NATIVE_BUILD
    bool frameReceived = false;
    unsigned long currentTime = micros();

    for (int i = 0; i < channelAmount_ && i < 6; ++i) {
        int value = ppm_.latestValidChannelValue(i + 1, -1);
        if (value != -1) {
            frameReceived = true;
            channels_[i] = value;
        }
    }

    if (frameReceived) {
        lastPPMUpdateTime_ = currentTime;
        ppmSignalLost_ = false;
    } else if (currentTime - lastPPMUpdateTime_ > 1000000) { // 1 second failsafe
        ppmSignalLost_ = true;
    }
#endif
}

int PPMReceiver::getChannel(int channelIdx) const {
    if (overrideActive_) {
        return (channelIdx >= 0 && channelIdx < 6) ? oChannels_[channelIdx] : 1500;
    }
    if (isSignalLost()) {
        // Safe defaults on signal loss matching sample code
        if (channelIdx == 2) return 1000; // Minimum Throttle
        return 1500; // Centers Roll, Pitch, Yaw
    }
    return (channelIdx >= 0 && channelIdx < 6) ? channels_[channelIdx] : 1500;
}

bool PPMReceiver::isSignalLost() const {
    if (overrideActive_) {
        return overrideSignalLost_;
    }
    return ppmSignalLost_;
}

void PPMReceiver::setOverride(int channelIdx, int value) {
    if (channelIdx >= 0 && channelIdx < 6) {
        oChannels_[channelIdx] = value;
    }
}
