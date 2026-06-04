# System Architecture & Folder Structure

This file documents the structural and design layout of the ESP32 Flight Controller C++ OOP Rewrite project.

---

## Directory Layout

```text
esp32_drone/
├── .pio/                 # PlatformIO intermediate build outputs
├── include/              # Public headers
│   ├── interfaces/       # Abstract Hardware Interfaces
│   │   ├── IIMU.h        # IMU Gyro/Accel interface
│   │   ├── IPPM.h        # PPM receiver interface
│   │   ├── IMotors.h     # ESC Motors output interface
│   │   └── IBattery.h    # Battery telemetry interface
│   ├── core/             # Platform-independent core algorithms
│   │   ├── PIDController.h
│   │   ├── KalmanFilter.h
│   │   └── FlightController.h
│   ├── hardware/         # ESP32 specific hardware headers
│   │   ├── MPU6050IMU.h
│   │   ├── PPMReceiver.h
│   │   ├── PWMESP32Motors.h
│   │   └── ADCBatteryMonitor.h
│   └── simulation/       # Simulation & Mock implementations
│       └── SimulatedHardware.h
├── src/                  # Source implementation files
│   ├── core/
│   │   ├── PIDController.cpp
│   │   ├── KalmanFilter.cpp
│   │   └── FlightController.cpp
│   ├── hardware/
│   │   ├── MPU6050IMU.cpp
│   │   ├── PPMReceiver.cpp
│   │   ├── PWMESP32Motors.cpp
│   │   └── ADCBatteryMonitor.cpp
│   └── main.cpp          # ESP32 Firmware Entry Point
├── tests/
│   └── tdd/              # Host-based doctest unit tests
│       ├── test_pid.cpp
│       ├── test_kalman.cpp
│       ├── test_flight_controller.cpp
│       └── test_simulation.cpp
├── platformio.ini        # PlatformIO Project Configuration
├── agent.md              # C++ Agent Guidelines & Programming Standards
├── architecture.md       # Architecture & Folder Structure (this file)
└── handoff.md            # Git & Handoff synchronization file
```

---

## Component Diagram

```mermaid
classDiagram
    class IIMU {
        <<interface>>
        +readSensor() void
        +getGyroRates(float& r, float& p, float& y) void
        +getAccAngles(float& r, float& p) void
        +setOverride(float rRate, float pRate, float yRate, float rAngle, float pAngle) void
    }

    class IPPM {
        <<interface>>
        +readChannels() void
        +getChannel(int idx) int
        +isSignalLost() bool
        +setOverride(int channel, int value) void
        +setSignalLostOverride(bool lost) void
    }

    class IMotors {
        <<interface>>
        +writeMotors(int m1, int m2, int m3, int m4) void
        +setOverride(int motorIdx, int value, bool active) void
    }

    class IBattery {
        <<interface>>
        +readVoltage() float
        +isLow() bool
        +setOverride(float voltage) void
    }

    IIMU <|-- MPU6050IMU
    IPPM <|-- PPMReceiver
    IMotors <|-- PWMESP32Motors
    IBattery <|-- ADCBatteryMonitor

    IIMU <|-- SimulatedIMU
    IPPM <|-- SimulatedPPMReceiver
    IMotors <|-- SimulatedMotors
    IBattery <|-- SimulatedBatteryMonitor

    class PIDController {
        +PIDController(float p, float i, float d)
        +update(float error, float prevError, float prevIterm, float dt) float
        +getIterm() float
        +getError() float
        +reset() void
    }

    class KalmanFilter {
        +KalmanFilter()
        +update(float state, float uncertainty, float input, float measurement, float dt) void
        +getState() float
        +getUncertainty() float
    }

    class FlightController {
        +FlightController(IIMU& imu, IPPM& ppm, IMotors& motors, IBattery& battery)
        +init() void
        +update(float dt) void
        +reset() void
    }

    FlightController --> IIMU
    FlightController --> IPPM
    FlightController --> IMotors
    FlightController --> IBattery
    FlightController --> PIDController
    FlightController --> KalmanFilter

---

## Hardware Connection Diagram

Below is the physical connection mapping for the ESP32 Flight Controller:

```text
                  +-----------------------------------+
                  |        ESP32 DEV MODULE           |
                  |                                   |
                  | [3V3] [GND] [RX2] [I2C] [SPI]     |
                  +---+-----+-----+----+----+---------+
                      |     |     |    |    |
   +------------------+     |     |    |    |
   | (3.3V Power)           |     |    |    |
   |                        |     |    |    |
   v                        v     |    |    v
+--+-------------+       +--+--+  |    |  +-+--------------+
| MPU6500 (SPI)  |       | GND |  |    |  | QMC5883L (I2C) |
|                |       +-----+  |    |  |                |
| VCC  <-> 3.3V  |                |    |  | VCC  <-> 3.3V  |
| GND  <-> GND   |                |    |  | GND  <-> GND   |
| SCK  <-> GPIO18|                |    |  | SCL  <-> GPIO22|
| MISO <-> GPIO19|                |    |  | SDA  <-> GPIO21|
| MOSI <-> GPIO23|                |    |  +----------------+
| CS   <-> GPIO5 |                |    |
+----------------+                |    |
                                  |    v
                                  |  +---------------------+
                                  |  | Flysky i-BUS RC RX  |
                                  |  |                     |
                                  |  | VCC  <-> 5V (BEC)   |
                                  |  | GND  <-> GND        |
                                  |  | Servo<-> RX2/GPIO16 |
                                  |  +---------------------+
                                  v
                       +----------+----------+
                       |   Battery Monitor   |
                       |                     |
   BAT+ [11.1V 3S] ----+--[ R1: 77.6k ]--+---+
                       |                 |
                       |                 +-----> GPIO33 (ADC)
                       |                 |
                       |   [ R2: 29.4k ] |
                       |                 |
   GND ----------------+-----------------+
                       |
                       +---------------------+
                                             |
                                             v
                                  +----------+----------+
                                  |   ESC & Motors      |
                                  |                     |
                                  |  M1 (Rear R): GPIO25|
                                  |  M2 (Front R):GPIO27|
                                  |  M3 (Front L):GPIO4 |
                                  |  M4 (Rear L): GPIO14|
                                  |                     |
                                  |  * Signal wires only|
                                  |  * Common GND with  |
                                  |    ESP32            |
                                  +---------------------+
```

### Pin Allocation Table

| Device | Device Pin | ESP32 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **MPU6500 IMU** | VCC | 3.3V | Power supply (3.3V) |
| | GND | GND | Ground |
| | SCK / SCL | GPIO 18 | VSPI Clock |
| | MISO / ADO | GPIO 19 | VSPI Master In Slave Out |
| | MOSI / SDA | GPIO 23 | VSPI Master Out Slave In |
| | CS / NCS | GPIO 5 | VSPI Chip Select |
| **QMC5883L** | VCC | 3.3V | Power supply (3.3V) |
| | GND | GND | Ground |
| | SCL | GPIO 22 | I2C Clock |
| | SDA | GPIO 21 | I2C Data |
| **i-BUS RX** | VCC | 5V / VIN | Power supply (from 5V BEC) |
| | GND | GND | Ground |
| | i-BUS Out | GPIO 16 (RX2) | Hardware Serial 2 RX |
| **Battery Monitor** | Divider Out | GPIO 33 | ADC Input (VMax ~ 3.05V for 12.6V Battery) |
| **LED Indicator**| Positive | GPIO 2 | Low battery indicator LED |
| **ESCs / Motors**| Motor 1 (Rear Right) | GPIO 25 | LEDC PWM Channel 0 (CCW / CW depending on mixing) |
| | Motor 2 (Front Right) | GPIO 27 | LEDC PWM Channel 1 |
| | Motor 3 (Front Left) | GPIO 4 | LEDC PWM Channel 2 |
| | Motor 4 (Rear Left) | GPIO 14 | LEDC PWM Channel 3 |

```
