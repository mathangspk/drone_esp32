#ifndef IBUS_RECEIVER_H
#define IBUS_RECEIVER_H

#include <Arduino.h>

class IBusReceiver {
private:
    static const uint8_t IBUS_BUFFSIZE = 32;
    static const uint8_t IBUS_MAX_CHANNELS = 14;
    static const uint16_t IBUS_LENGTH = 0x20;
    static const uint8_t IBUS_COMMAND = 0x40;
    
    HardwareSerial* ibus_serial;
    uint8_t buffer[IBUS_BUFFSIZE];
    uint16_t channel_data[IBUS_MAX_CHANNELS];
    uint8_t buffer_index;
    unsigned long last_read_time;

    bool read_one_byte(uint8_t* byte);
    bool verify_checksum();
    void update_channels();

public:
    IBusReceiver(HardwareSerial* serial);
    void begin();
    bool read_channel_data();
    uint16_t get_channel(uint8_t channel_num) const;
    
    // Helper functions to get specific channels
    uint16_t get_throttle() const { return get_channel(2); }
    uint16_t get_roll() const { return get_channel(0); }
    uint16_t get_pitch() const { return get_channel(1); }
    uint16_t get_yaw() const { return get_channel(3); }
    uint16_t get_aux1() const { return get_channel(4); }
    uint16_t get_aux2() const { return get_channel(5); }
};

#endif // IBUS_RECEIVER_H 