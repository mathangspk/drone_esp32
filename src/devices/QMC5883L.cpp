#include "devices/QMC5883L.h"
#include <math.h>

QMC5883L::QMC5883L() {}

bool QMC5883L::begin() {
    Wire.begin();
    delay(10);
    
    // Reset
    if(!writeRegister(REG_CONTROL2, 0x80)) {
        return false;
    }
    delay(10);
    
    // Cấu hình ban đầu với độ nhạy cao nhất
    setRange(RNG_2G);  // ±2G để có độ nhạy tốt hơn
    setODR(ODR_50HZ);  // 50Hz là đủ cho ứng dụng drone
    setOSR(OSR_512);   // Oversampling cao nhất để giảm nhiễu
    
    // Set/Reset period
    if(!writeRegister(REG_SET_RESET, 0x01)) {
        return false;
    }
    
    // Mode liên tục
    uint8_t config = MODE_CONTINUOUS;
    if(!writeRegister(REG_CONTROL1, config)) {
        return false;
    }
    
    delay(50); // Đợi measurement đầu tiên hoàn thành
    return true;
}

void QMC5883L::setRange(uint8_t range) {
    mag_range = (range == RNG_8G) ? 8.0f : 2.0f;
    uint8_t current;
    readRegisters(REG_CONTROL1, &current, 1);
    current &= ~0x30;  // Clear range bits
    current |= range;  // Set new range
    writeRegister(REG_CONTROL1, current);
}

void QMC5883L::setOSR(uint8_t osr) {
    uint8_t current;
    readRegisters(REG_CONTROL1, &current, 1);
    current &= ~0xC0;  // Clear OSR bits
    current |= osr;    // Set new OSR
    writeRegister(REG_CONTROL1, current);
}

void QMC5883L::setODR(uint8_t odr) {
    uint8_t current;
    readRegisters(REG_CONTROL1, &current, 1);
    current &= ~0x0C;  // Clear ODR bits
    current |= odr;    // Set new ODR
    writeRegister(REG_CONTROL1, current);
}

void QMC5883L::readMag(float &mx, float &my, float &mz) {
    // Đọc trạng thái
    uint8_t status;
    readRegisters(REG_STATUS, &status, 1);
    
    // Kiểm tra bit DRDY (Data Ready) và DOR (Data Overrun)
    if(!(status & 0x01) || (status & 0x02)) {
        Serial.println("Data not ready or overrun");
        return; // Dữ liệu chưa sẵn sàng hoặc bị overrun
    }
    
    // Đọc tất cả dữ liệu cùng lúc
    uint8_t buffer[6];
    if(!readRegisters(REG_DATA_X_LSB, buffer, 6)) {
        Serial.println("Failed to read magnetometer data");
        return;
    }
    
    // Chuyển đổi từ bytes sang int16_t
    int16_t x = (int16_t)(buffer[1] << 8 | buffer[0]);
    int16_t y = (int16_t)(buffer[3] << 8 | buffer[2]);
    int16_t z = (int16_t)(buffer[5] << 8 | buffer[4]);
    
    // In ra giá trị thô để debug
    //Serial.printf("Raw mag data: X=%d Y=%d Z=%d\n", x, y, z);
    
    // Áp dụng hiệu chuẩn
    mx = (float)x * mag_scale[0] + mag_offset[0];
    my = (float)y * mag_scale[1] + mag_offset[1];
    mz = (float)z * mag_scale[2] + mag_offset[2];
    
    // Chuyển đổi sang Gauss (thang đo phụ thuộc vào cấu hình)
    const float conversion = mag_range / 32768.0f;
    mx *= conversion;
    my *= conversion;
    mz *= conversion;
    
    // In ra giá trị sau khi chuyển đổi
    //Serial.printf("Calibrated mag data (Gauss): X=%.3f Y=%.3f Z=%.3f\n", mx, my, mz);
}

float QMC5883L::getHeading() {
    float mx, my, mz;
    readMag(mx, my, mz);
    
    float heading = atan2(my, mx);
    
    // Chuyển từ radian sang độ
    heading *= 180.0 / M_PI;
    
    // Đảm bảo heading nằm trong khoảng 0-360
    if(heading < 0) {
        heading += 360;
    }
    
    return heading;
}

void QMC5883L::calibrate() {
    const int samples = 500;
    float min_x = 0, max_x = 0;
    float min_y = 0, max_y = 0;
    float min_z = 0, max_z = 0;
    float mx, my, mz;
    
    Serial.println("Bắt đầu hiệu chuẩn QMC5883L...");
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
    
    float range_x = (max_x - min_x) / 2;
    float range_y = (max_y - min_y) / 2;
    float range_z = (max_z - min_z) / 2;
    float avg_range = (range_x + range_y + range_z) / 3;
    
    mag_scale[0] = avg_range / range_x;
    mag_scale[1] = avg_range / range_y;
    mag_scale[2] = avg_range / range_z;
    
    Serial.println("Hiệu chuẩn hoàn tất!");
    Serial.printf("Min: X=%.2f Y=%.2f Z=%.2f\n", min_x, min_y, min_z);
    Serial.printf("Max: X=%.2f Y=%.2f Z=%.2f\n", max_x, max_y, max_z);
    Serial.printf("Offset: X=%.2f Y=%.2f Z=%.2f\n", mag_offset[0], mag_offset[1], mag_offset[2]);
    Serial.printf("Scale: X=%.2f Y=%.2f Z=%.2f\n", mag_scale[0], mag_scale[1], mag_scale[2]);
}

bool QMC5883L::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(reg);
    Wire.write(value);
    uint8_t error = Wire.endTransmission();
    if(error != 0) {
        Serial.printf("I2C write error: %d\n", error);
        return false;
    }
    return true;
}

bool QMC5883L::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t count) {
    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(reg);
    if(Wire.endTransmission() != 0) {
        return false;
    }
    
    uint8_t bytesRead = Wire.requestFrom(QMC5883L_ADDR, count);
    if(bytesRead != count) {
        return false;
    }
    
    for(uint8_t i = 0; i < count && Wire.available(); i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

int16_t QMC5883L::readRawAxis(uint8_t lsbRegister, uint8_t msbRegister) {
    uint8_t buffer[2];
    readRegisters(lsbRegister, buffer, 2);
    return (int16_t)((buffer[1] << 8) | buffer[0]);
} 

// Thêm vào QMC5883L.cpp
bool QMC5883L::dataReady() {
    uint8_t status;
    if (!readRegisters(REG_STATUS, &status, 1)) return false;
    return (status & 0x01) && !(status & 0x02); // DRDY=1, DOR=0
}