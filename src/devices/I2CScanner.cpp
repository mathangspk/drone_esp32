#include "devices/I2CScanner.h"

// Định nghĩa danh sách các thiết bị I2C phổ biến với DeviceType
const I2CScanner::CommonDevice I2CScanner::commonDevices[] = {
    {0x68, "MPU6050/MPU6500", "6-axis motion sensor", DEVICE_MPU6500},
    {0x69, "MPU6050/MPU6500", "6-axis motion sensor (AD0=1)", DEVICE_MPU6500},
    {0x76, "BME280", "Temperature, pressure, humidity sensor", DEVICE_BME280},
    {0x77, "BME280", "Temperature, pressure, humidity sensor (SDO=1)", DEVICE_BME280},
    {0x1E, "HMC5883L", "3-axis magnetometer", DEVICE_HMC5883L},
    {0x0D, "QMC5883L", "3-axis magnetometer", DEVICE_QMC5883L},
    {0x48, "ADS1115", "16-bit ADC", DEVICE_ADS1115},
    {0x49, "ADS1115", "16-bit ADC (A1=1)", DEVICE_ADS1115},
    {0x4A, "ADS1115", "16-bit ADC (A0=1)", DEVICE_ADS1115},
    {0x4B, "ADS1115", "16-bit ADC (A0=1, A1=1)", DEVICE_ADS1115},
    {0x27, "PCF8574", "8-bit I/O expander", DEVICE_PCF8574},
    {0x20, "MCP23017", "16-bit I/O expander", DEVICE_MCP23017},
    {0x21, "MCP23017", "16-bit I/O expander (A0=1)", DEVICE_MCP23017},
    {0x22, "MCP23017", "16-bit I/O expander (A1=1)", DEVICE_MCP23017},
    {0x23, "MCP23017", "16-bit I/O expander (A0=1, A1=1)", DEVICE_MCP23017},
    {0x24, "MCP23017", "16-bit I/O expander (A2=1)", DEVICE_MCP23017},
    {0x25, "MCP23017", "16-bit I/O expander (A0=1, A2=1)", DEVICE_MCP23017},
    {0x26, "MCP23017", "16-bit I/O expander (A1=1, A2=1)", DEVICE_MCP23017},
    {0x27, "MCP23017", "16-bit I/O expander (A0=1, A1=1, A2=1)", DEVICE_MCP23017},
    {0x3C, "SSD1306", "OLED display", DEVICE_SSD1306},
    {0x3D, "SSD1306", "OLED display (SA0=1)", DEVICE_SSD1306},
    {0x50, "EEPROM", "24CXX EEPROM", DEVICE_EEPROM},
    {0x51, "EEPROM", "24CXX EEPROM (A0=1)", DEVICE_EEPROM},
    {0x52, "EEPROM", "24CXX EEPROM (A1=1)", DEVICE_EEPROM},
    {0x53, "EEPROM", "24CXX EEPROM (A0=1, A1=1)", DEVICE_EEPROM},
    {0x54, "EEPROM", "24CXX EEPROM (A2=1)", DEVICE_EEPROM},
    {0x55, "EEPROM", "24CXX EEPROM (A0=1, A2=1)", DEVICE_EEPROM},
    {0x56, "EEPROM", "24CXX EEPROM (A1=1, A2=1)", DEVICE_EEPROM},
    {0x57, "EEPROM", "24CXX EEPROM (A0=1, A1=1, A2=1)", DEVICE_EEPROM},
    {0x70, "PCA9685", "16-channel PWM controller", DEVICE_PCA9685},
    {0x71, "PCA9685", "16-channel PWM controller (A0=1)", DEVICE_PCA9685},
    {0x72, "PCA9685", "16-channel PWM controller (A1=1)", DEVICE_PCA9685},
    {0x73, "PCA9685", "16-channel PWM controller (A0=1, A1=1)", DEVICE_PCA9685},
    {0x74, "PCA9685", "16-channel PWM controller (A2=1)", DEVICE_PCA9685},
    {0x75, "PCA9685", "16-channel PWM controller (A0=1, A2=1)", DEVICE_PCA9685},
    {0x76, "PCA9685", "16-channel PWM controller (A1=1, A2=1)", DEVICE_PCA9685},
    {0x77, "PCA9685", "16-channel PWM controller (A0=1, A1=1, A2=1)", DEVICE_PCA9685}
};

const uint8_t I2CScanner::commonDeviceCount = sizeof(commonDevices) / sizeof(commonDevices[0]);

I2CScanner::I2CScanner() : deviceCount(0), requiredDeviceCount(0) {
    // Khởi tạo mảng địa chỉ
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        foundAddresses[i] = 0;
    }
    
    // Khởi tạo danh sách thiết bị cần thiết
    initializeRequiredDevicesList();
    
    // Khởi tạo callbacks
    for (uint8_t i = 0; i < 10; i++) {
        deviceCallbacks[i] = nullptr;
    }
    errorHandler = nullptr;
    successHandler = nullptr;
}

void I2CScanner::initializeRequiredDevicesList() {
    // Định nghĩa các thiết bị I2C cần thiết cho drone
    // MPU6500 kết nối qua SPI nên không có trong danh sách I2C
    requiredDevices[0] = {0x76, DEVICE_BME280, "BME280", "Temperature, pressure, humidity sensor", false, false, false};
    requiredDevices[1] = {0x0D, DEVICE_QMC5883L, "QMC5883L", "3-axis magnetometer", false, false, false};
    requiredDevices[2] = {0x1E, DEVICE_HMC5883L, "HMC5883L", "3-axis magnetometer (backup)", false, false, false};
    requiredDeviceCount = 3;
}

void I2CScanner::scanAndPrint() {
    Serial.println("\n=== Quét I2C bus ===");
    
    deviceCount = scan();
    
    if (deviceCount == 0) {
        Serial.println("Không tìm thấy thiết bị I2C nào!");
    } else {
        Serial.printf("Tìm thấy %d thiết bị I2C:\n", deviceCount);
        
        for (uint8_t i = 0; i < deviceCount; i++) {
            printDeviceInfo(foundAddresses[i]);
        }
        
        // Kiểm tra các thiết bị phổ biến
        Serial.println("\n=== Kiểm tra thiết bị phổ biến ===");
        checkCommonDevices();
        
        // Kiểm tra thiết bị cần thiết
        Serial.println("\n=== Kiểm tra thiết bị cần thiết ===");
        checkRequiredDevices();
    }
    
    Serial.println("=== Quét I2C hoàn tất ===\n");
}

uint8_t I2CScanner::scan() {
    deviceCount = 0;
    
    for (uint8_t addr = 0; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            if (deviceCount < MAX_DEVICES) {
                foundAddresses[deviceCount] = addr;
                deviceCount++;
            }
        }
    }
    
    return deviceCount;
}

bool I2CScanner::isDevicePresent(uint8_t address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    return (error == 0);
}

uint8_t* I2CScanner::getFoundAddresses() {
    return foundAddresses;
}

uint8_t I2CScanner::getDeviceCount() const {
    return deviceCount;
}

void I2CScanner::printDeviceInfo(uint8_t address) {
    Serial.printf("0x%02X", address);
    
    // Tìm tên thiết bị trong danh sách phổ biến
    bool found = false;
    for (uint8_t i = 0; i < commonDeviceCount; i++) {
        if (commonDevices[i].address == address) {
            Serial.printf(" - %s: %s", commonDevices[i].name, commonDevices[i].description);
            found = true;
            break;
        }
    }
    
    if (!found) {
        Serial.print(" - Unknown device");
    }
    
    Serial.println();
}

void I2CScanner::checkCommonDevices() {
    for (uint8_t i = 0; i < commonDeviceCount; i++) {
        if (isDevicePresent(commonDevices[i].address)) {
            Serial.printf("✓ %s (0x%02X): %s\n", 
                         commonDevices[i].name, 
                         commonDevices[i].address, 
                         commonDevices[i].description);
        }
    }
}

bool I2CScanner::checkRequiredDevices() {
    bool allRequiredFound = true;
    
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        bool found = isDevicePresent(requiredDevices[i].address);
        requiredDevices[i].isWorking = found;
        
        if (requiredDevices[i].isRequired) {
            if (found) {
                Serial.printf("✓ %s (0x%02X): %s - REQUIRED\n", 
                             requiredDevices[i].name, 
                             requiredDevices[i].address, 
                             requiredDevices[i].description);
            } else {
                Serial.printf("✗ %s (0x%02X): %s - REQUIRED (NOT FOUND!)\n", 
                             requiredDevices[i].name, 
                             requiredDevices[i].address, 
                             requiredDevices[i].description);
                allRequiredFound = false;
                
                if (errorHandler) {
                    errorHandler(requiredDevices[i].type, "Required device not found");
                }
            }
        } else {
            if (found) {
                Serial.printf("✓ %s (0x%02X): %s - OPTIONAL\n", 
                             requiredDevices[i].name, 
                             requiredDevices[i].address, 
                             requiredDevices[i].description);
            } else {
                Serial.printf("- %s (0x%02X): %s - OPTIONAL (not found)\n", 
                             requiredDevices[i].name, 
                             requiredDevices[i].address, 
                             requiredDevices[i].description);
            }
        }
    }
    
    return allRequiredFound;
}

bool I2CScanner::initializeDevices() {
    bool allInitialized = true;
    
    Serial.println("\n=== Khởi tạo thiết bị ===");
    
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].isWorking && !requiredDevices[i].isInitialized) {
            if (deviceCallbacks[i]) {
                bool success = deviceCallbacks[i]();
                requiredDevices[i].isInitialized = success;
                
                if (success) {
                    Serial.printf("✓ %s: Khởi tạo thành công\n", requiredDevices[i].name);
                    if (successHandler) {
                        successHandler(requiredDevices[i].type, "Device initialized successfully");
                    }
                } else {
                    Serial.printf("✗ %s: Khởi tạo thất bại\n", requiredDevices[i].name);
                    allInitialized = false;
                    if (errorHandler) {
                        errorHandler(requiredDevices[i].type, "Device initialization failed");
                    }
                }
            } else {
                Serial.printf("- %s: Không có callback khởi tạo\n", requiredDevices[i].name);
            }
        }
    }
    
    Serial.println("=== Khởi tạo thiết bị hoàn tất ===\n");
    return allInitialized;
}

void I2CScanner::printDeviceStatus() {
    Serial.println("\n=== Trạng thái thiết bị ===");
    
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        const char* status = "UNKNOWN";
        if (requiredDevices[i].isWorking) {
            status = requiredDevices[i].isInitialized ? "WORKING" : "FOUND";
        } else {
            status = requiredDevices[i].isRequired ? "MISSING" : "NOT FOUND";
        }
        
        Serial.printf("%s (0x%02X): %s %s\n", 
                     requiredDevices[i].name, 
                     requiredDevices[i].address, 
                     status,
                     requiredDevices[i].isRequired ? "(REQUIRED)" : "(OPTIONAL)");
    }
    
    Serial.println("=== Trạng thái thiết bị hoàn tất ===\n");
}

bool I2CScanner::isDeviceWorking(DeviceType deviceType) {
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].type == deviceType) {
            return requiredDevices[i].isWorking;
        }
    }
    return false;
}

bool I2CScanner::isDeviceInitialized(DeviceType deviceType) {
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].type == deviceType) {
            return requiredDevices[i].isInitialized;
        }
    }
    return false;
}

uint8_t I2CScanner::getDeviceAddress(DeviceType deviceType) {
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].type == deviceType) {
            return requiredDevices[i].address;
        }
    }
    return 0;
}

void I2CScanner::setDeviceCallback(DeviceType deviceType, std::function<bool()> initCallback) {
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].type == deviceType) {
            deviceCallbacks[i] = initCallback;
            break;
        }
    }
}

void I2CScanner::setErrorHandler(std::function<void(DeviceType, const char*)> handler) {
    errorHandler = handler;
}

void I2CScanner::setSuccessHandler(std::function<void(DeviceType, const char*)> handler) {
    successHandler = handler;
}

DeviceType I2CScanner::getDeviceTypeByAddress(uint8_t address) {
    for (uint8_t i = 0; i < commonDeviceCount; i++) {
        if (commonDevices[i].address == address) {
            return commonDevices[i].type;
        }
    }
    return DEVICE_NONE;
}

const char* I2CScanner::getDeviceNameByType(DeviceType type) {
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].type == type) {
            return requiredDevices[i].name;
        }
    }
    return "Unknown";
}

void I2CScanner::updateDeviceStatus(DeviceType type, bool isWorking) {
    for (uint8_t i = 0; i < requiredDeviceCount; i++) {
        if (requiredDevices[i].type == type) {
            requiredDevices[i].isWorking = isWorking;
            break;
        }
    }
} 