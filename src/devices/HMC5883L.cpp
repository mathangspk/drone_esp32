#include "devices/HMC5883L.h"
#include <math.h>

HMC5883L::HMC5883L() {}

bool HMC5883L::begin() {
    Wire.begin();
    delay(10); // Đợi cảm biến khởi động

    // Quét địa chỉ I2C
    Serial.println("Quét địa chỉ I2C...");
    bool found = false;
    uint8_t address = 0;
    
    for(uint8_t addr = 0; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        
        if(error == 0) {
            Serial.printf("Tìm thấy thiết bị I2C tại địa chỉ: 0x%02X\n", addr);
            // Thử đọc ID register để xác định HMC5883L
            Wire.beginTransmission(addr);
            Wire.write(0x0A);
            if(Wire.endTransmission() == 0) {
                Wire.requestFrom(addr, (uint8_t)3);
                if(Wire.available() == 3) {
                    char id[3];
                    for(int i = 0; i < 3; i++) {
                        id[i] = Wire.read();
                    }
                    if(id[0] == 'H' && id[1] == '4' && id[2] == '3') {
                        address = addr;
                        found = true;
                        Serial.printf("Xác nhận HMC5883L tại địa chỉ 0x%02X\n", addr);
                        break;
                    }
                }
            }
        }
    }

    if(!found) {
        Serial.println("Không tìm thấy HMC5883L! Thử với địa chỉ mặc định 0x0D");
        address = 0x0D;
    }

    _address = address;
    
    // Cấu hình cảm biến
    // Sample Rate = 15Hz, Normal measurement mode
    if(!writeRegister(CONFIG_A, 0x70)) return false;
    
    // Gain = 1090 LSB/Gauss
    if(!writeRegister(CONFIG_B, 0x20)) return false;
    
    // Continuous measurement mode
    if(!writeRegister(MODE, 0x00)) return false;
    
    delay(6); // Đợi measurement đầu tiên hoàn thành
    return true;
}

void HMC5883L::readMag(float &mx, float &my, float &mz) {
    uint8_t buffer[6];
    bool success = readRegisters(DATA_X_MSB, buffer, 6);
    
    if (!success) {
        mx = my = mz = 0;
        return;
    }
    
    // Chuyển đổi raw data thành giá trị từ trường (mG)
    int16_t x = (buffer[0] << 8) | buffer[1];
    int16_t z = (buffer[2] << 8) | buffer[3];
    int16_t y = (buffer[4] << 8) | buffer[5];
    
    // Áp dụng hiệu chuẩn và chuyển đổi sang đơn vị Gauss
    mx = (float)x * mag_scale[0] + mag_offset[0];
    my = (float)y * mag_scale[1] + mag_offset[1];
    mz = (float)z * mag_scale[2] + mag_offset[2];
    
    // Chuyển đổi từ raw sang Gauss (thang đo ±1.3Ga)
    mx /= 1090.0f;
    my /= 1090.0f;
    mz /= 1090.0f;
}

float HMC5883L::getHeading() {
    float mx, my, mz;
    readMag(mx, my, mz);
    
    // Tính góc heading từ các thành phần x và y
    float heading = atan2(my, mx);
    
    // Chuyển đổi từ radian sang độ
    heading *= 180.0 / M_PI;
    
    // Đảm bảo heading nằm trong khoảng 0-360
    if (heading < 0) {
        heading += 360;
    }
    
    return heading;
}

void HMC5883L::calibrate() {
    const int samples = 500;
    float min_x = 0, max_x = 0;
    float min_y = 0, max_y = 0;
    float min_z = 0, max_z = 0;
    float mx, my, mz;
    
    Serial.println("Bắt đầu hiệu chuẩn HMC5883L...");
    Serial.println("Xoay cảm biến theo mọi hướng trong 10 giây");
    
    // Thu thập dữ liệu
    for(int i = 0; i < samples; i++) {
        readMag(mx, my, mz);
        
        if(i == 0) {
            min_x = max_x = mx;
            min_y = max_y = my;
            min_z = max_z = mz;
        } else {
            min_x = min(min_x, mx);
            min_y = min(min_y, my);
            min_z = min(min_z, mz);
            max_x = max(max_x, mx);
            max_y = max(max_y, my);
            max_z = max(max_z, mz);
        }
        delay(20);
    }
    
    // Tính offset và scale
    mag_offset[0] = (min_x + max_x) / 2;
    mag_offset[1] = (min_y + max_y) / 2;
    mag_offset[2] = (min_z + max_z) / 2;
    
    mag_scale[0] = 2 / (max_x - min_x);
    mag_scale[1] = 2 / (max_y - min_y);
    mag_scale[2] = 2 / (max_z - min_z);
    
    Serial.println("Hiệu chuẩn hoàn tất!");
    Serial.printf("Offset: X=%.2f Y=%.2f Z=%.2f\n", mag_offset[0], mag_offset[1], mag_offset[2]);
    Serial.printf("Scale: X=%.2f Y=%.2f Z=%.2f\n", mag_scale[0], mag_scale[1], mag_scale[2]);
}

bool HMC5883L::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool HMC5883L::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t count) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    if(Wire.endTransmission() != 0) {
        return false;
    }
    
    uint8_t bytesRead = Wire.requestFrom(_address, count);
    if(bytesRead != count) {
        return false;
    }
    
    for (uint8_t i = 0; i < count && Wire.available(); i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
} 