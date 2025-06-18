#include "receiver/IBusReceiver.h"

IBusReceiver::IBusReceiver(HardwareSerial* serial) : 
    ibus_serial(serial),
    buffer_index(0),
    last_read_time(0) {
    memset(buffer, 0, IBUS_BUFFSIZE);
    memset(channel_data, 0, IBUS_MAX_CHANNELS * sizeof(uint16_t));
}

void IBusReceiver::begin() {
    ibus_serial->begin(115200);
}

bool IBusReceiver::read_one_byte(uint8_t* byte) {
    if (ibus_serial->available()) {
        *byte = ibus_serial->read();
        return true;
    }
    return false;
}

bool IBusReceiver::verify_checksum() {
    uint16_t checksum = 0xFFFF;
    for (uint8_t i = 0; i < 30; i++) {
        checksum -= buffer[i];
    }
    
    uint16_t received_checksum = (buffer[31] << 8) | buffer[30];
    return checksum == received_checksum;
}

void IBusReceiver::update_channels() {
    for (uint8_t i = 0; i < IBUS_MAX_CHANNELS; i++) {
        uint8_t index = i * 2 + 2;
        channel_data[i] = (buffer[index + 1] << 8) | buffer[index];
    }
}

bool IBusReceiver::read_channel_data() {
    uint8_t byte;
    unsigned long current_time = millis();
    
    // Reset buffer if no data received for 10ms
    if (current_time - last_read_time > 10) {
        buffer_index = 0;
    }
    
    while (read_one_byte(&byte)) {
        last_read_time = current_time;
        
        if (buffer_index == 0 && byte != IBUS_LENGTH) {
            continue;
        }
        
        if (buffer_index == 1 && byte != IBUS_COMMAND) {
            buffer_index = 0;
            continue;
        }
        
        buffer[buffer_index++] = byte;
        
        if (buffer_index == IBUS_BUFFSIZE) {
            buffer_index = 0;
            if (verify_checksum()) {
                update_channels();
                return true;
            }
        }
    }
    
    return false;
}

uint16_t IBusReceiver::get_channel(uint8_t channel_num) const {
    if (channel_num < IBUS_MAX_CHANNELS) {
        return channel_data[channel_num];
    }
    return 0;
} 