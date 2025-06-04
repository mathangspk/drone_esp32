#include <Arduino.h>
#include "IBusReceiver.h"

// Khởi tạo Serial2 cho IBus (ESP32 - GPIO16 RX2)
HardwareSerial ibus_serial(2);
IBusReceiver receiver(&ibus_serial);

void setup() {
    // Khởi tạo Serial để debug
    Serial.begin(115200);
    
    // Khởi tạo IBus receiver trên GPIO16
    receiver.begin();
}

void loop() {
    // Đọc dữ liệu từ receiver
    if (receiver.read_channel_data()) {
        // In ra các giá trị kênh
        Serial.print("Throttle: "); Serial.print(receiver.get_throttle());
        Serial.print(" Roll: "); Serial.print(receiver.get_roll());
        Serial.print(" Pitch: "); Serial.print(receiver.get_pitch());
        Serial.print(" Yaw: "); Serial.print(receiver.get_yaw());
        Serial.print(" AUX1: "); Serial.print(receiver.get_aux1());
        Serial.print(" AUX2: "); Serial.println(receiver.get_aux2());
    }
    
    // Hoặc bạn có thể đọc kênh cụ thể
    uint16_t throttle = receiver.get_channel(2);  // Kênh 3 (throttle)
    
    // Delay một chút để không in quá nhanh
    delay(100);
} 