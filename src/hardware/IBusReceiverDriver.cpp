#include "hardware/IBusReceiverDriver.h"

#ifndef NATIVE_BUILD
IBusReceiverDriver::IBusReceiverDriver(HardwareSerial* serial) : serial_(serial) {
    memset(buffer_, 0, 32);
    for (int i = 0; i < 14; ++i) {
        channels_[i] = (i == 2) ? 1000 : 1500;
    }
}

void IBusReceiverDriver::begin() {
    serial_->begin(115200, SERIAL_8N1, 16, -1); // RX2 on pin 16
}

bool IBusReceiverDriver::readByte(uint8_t* byte) {
    if (serial_->available()) {
        *byte = serial_->read();
        return true;
    }
    return false;
}

bool IBusReceiverDriver::verifyChecksum() {
    uint16_t checksum = 0xFFFF;
    for (uint8_t i = 0; i < 30; i++) {
        checksum -= buffer_[i];
    }
    uint16_t received = (buffer_[31] << 8) | buffer_[30];
    return checksum == received;
}

void IBusReceiverDriver::updateChannels() {
    for (uint8_t i = 0; i < 14; i++) {
        uint8_t idx = i * 2 + 2;
        channels_[i] = (buffer_[idx + 1] << 8) | buffer_[idx];
    }
}

void IBusReceiverDriver::readChannels() {
    if (oActive_) return;
    uint8_t val;
    unsigned long now = millis();
    if (now - lastReadTime_ > 10) {
        bufIndex_ = 0;
    }

    while (readByte(&val)) {
        lastReadTime_ = now;
        if (bufIndex_ == 0 && val != 0x20) continue;
        if (bufIndex_ == 1 && val != 0x40) {
            bufIndex_ = 0;
            continue;
        }
        buffer_[bufIndex_++] = val;
        if (bufIndex_ == 32) {
            bufIndex_ = 0;
            if (verifyChecksum()) {
                updateChannels();
                signalLost_ = false;
            }
        }
    }
    // Timeout of 1000ms is signal loss
    if (millis() - lastReadTime_ > 1000) {
        signalLost_ = true;
    }
}
#else
IBusReceiverDriver::IBusReceiverDriver(HardwareSerial* serial) : serial_(serial) {
    for (int i = 0; i < 14; ++i) channels_[i] = (i == 2) ? 1000 : 1500;
}
void IBusReceiverDriver::begin() {}
bool IBusReceiverDriver::readByte(uint8_t*) { return false; }
bool IBusReceiverDriver::verifyChecksum() { return true; }
void IBusReceiverDriver::updateChannels() {}
void IBusReceiverDriver::readChannels() {}
#endif

int IBusReceiverDriver::getChannel(int idx) const {
    if (oActive_) {
        return (idx >= 0 && idx < 6) ? oChannels_[idx] : 1500;
    }
    if (isSignalLost()) {
        return (idx == 2) ? 1000 : 1500;
    }
    return (idx >= 0 && idx < 14) ? channels_[idx] : 1500;
}

bool IBusReceiverDriver::isSignalLost() const {
    return oActive_ ? oSignalLost_ : signalLost_;
}

void IBusReceiverDriver::setOverride(int idx, int value) {
    if (idx >= 0 && idx < 6) {
        oChannels_[idx] = value;
    }
}
