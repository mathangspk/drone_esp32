#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Arduino.h>
#include <Wire.h>
#include <functional>

// Enum cho các loại thiết bị quan trọng
enum DeviceType {
    DEVICE_NONE = 0,
    DEVICE_MPU6500,
    DEVICE_BME280,
    DEVICE_QMC5883L,
    DEVICE_HMC5883L,
    DEVICE_ADS1115,
    DEVICE_SSD1306,
    DEVICE_EEPROM,
    DEVICE_PCA9685,
    DEVICE_MCP23017,
    DEVICE_PCF8574
};

// Struct lưu thông tin thiết bị
struct DeviceInfo {
    uint8_t address;
    DeviceType type;
    const char* name;
    const char* description;
    bool isRequired;
    bool isInitialized;
    bool isWorking;
};

class I2CScanner {
public:
    I2CScanner();
    
    // === Các phương thức quét cơ bản ===
    void scanAndPrint();
    uint8_t scan();
    bool isDevicePresent(uint8_t address);
    uint8_t* getFoundAddresses();
    uint8_t getDeviceCount() const;
    void printDeviceInfo(uint8_t address);
    void checkCommonDevices();

    // === Các phương thức kiểm tra và khởi tạo thiết bị ===
    bool checkRequiredDevices();
    bool initializeDevices();
    void printDeviceStatus();
    bool isDeviceWorking(DeviceType deviceType);
    bool isDeviceInitialized(DeviceType deviceType);
    uint8_t getDeviceAddress(DeviceType deviceType);
    
    // === Các phương thức tiện ích ===
    void setDeviceCallback(DeviceType deviceType, std::function<bool()> initCallback);
    void setErrorHandler(std::function<void(DeviceType, const char*)> errorHandler);
    void setSuccessHandler(std::function<void(DeviceType, const char*)> successHandler);

private:
    static const uint8_t MAX_DEVICES = 32;
    uint8_t foundAddresses[MAX_DEVICES];
    uint8_t deviceCount;
    
    // Danh sách thiết bị cần thiết cho drone
    DeviceInfo requiredDevices[10];
    uint8_t requiredDeviceCount;
    
    // Callbacks
    std::function<bool()> deviceCallbacks[10];
    std::function<void(DeviceType, const char*)> errorHandler;
    std::function<void(DeviceType, const char*)> successHandler;
    
    // Danh sách các thiết bị I2C phổ biến
    struct CommonDevice {
        uint8_t address;
        const char* name;
        const char* description;
        DeviceType type;
    };
    
    static const CommonDevice commonDevices[];
    static const uint8_t commonDeviceCount;
    
    // Helper methods
    void initializeRequiredDevicesList();
    DeviceType getDeviceTypeByAddress(uint8_t address);
    const char* getDeviceNameByType(DeviceType type);
    void updateDeviceStatus(DeviceType type, bool isWorking);
};

#endif // I2C_SCANNER_H 