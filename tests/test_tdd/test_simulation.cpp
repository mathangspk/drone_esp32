#include "doctest.h"
#include "simulation/SimulatedHardware.h"

TEST_CASE("SimulatedHardware overrides and telemetry inject") {
    SUBCASE("Simulated IMU overrides") {
        SimulatedIMU imu;
        float rRate, pRate, yRate, rAngle, pAngle;
        
        imu.readSensor();
        imu.getGyroRates(rRate, pRate, yRate);
        imu.getAccAngles(rAngle, pAngle);
        CHECK_EQ(rRate, 0.0f);
        
        imu.setOverride(12.5f, -8.2f, 1.5f, 15.0f, -10.0f);
        imu.setOverrideActive(true);
        imu.readSensor();
        imu.getGyroRates(rRate, pRate, yRate);
        imu.getAccAngles(rAngle, pAngle);
        
        CHECK_EQ(rRate, 12.5f);
        CHECK_EQ(pRate, -8.2f);
        CHECK_EQ(yRate, 1.5f);
        CHECK_EQ(rAngle, 15.0f);
        CHECK_EQ(pAngle, -10.0f);
    }

    SUBCASE("Simulated PPM signal loss and overrides") {
        SimulatedPPMReceiver ppm;
        
        ppm.readChannels();
        CHECK_EQ(ppm.getChannel(2), 1000); // default throttle idle
        CHECK_EQ(ppm.getChannel(0), 1500); // default roll center
        CHECK_FALSE(ppm.isSignalLost());
        
        // Override channels
        ppm.setOverride(2, 1850);
        ppm.setOverrideActive(true);
        ppm.readChannels();
        CHECK_EQ(ppm.getChannel(2), 1850);
        
        // Signal loss inject
        ppm.setSignalLostOverride(true);
        CHECK(ppm.isSignalLost());
    }

    SUBCASE("Simulated Battery and low-voltage trigger") {
        SimulatedBatteryMonitor battery;
        
        CHECK_EQ(battery.readVoltage(), 11.1f);
        CHECK_FALSE(battery.isLow());
        
        battery.setOverride(8.5f);
        battery.setOverrideActive(true);
        CHECK_EQ(battery.readVoltage(), 8.5f);
        CHECK(battery.isLow());
    }

    SUBCASE("Simulated Motor command tracking and output override") {
        SimulatedMotors motors;
        
        motors.writeMotors(1200, 1300, 1400, 1500);
        CHECK_EQ(motors.getMotorOutput(0), 1200);
        CHECK_EQ(motors.getMotorOutput(3), 1500);
        
        // Override motor 1 output
        motors.setOverride(0, 1000, true);
        CHECK_EQ(motors.getMotorOutput(0), 1000); // overridden
        CHECK_EQ(motors.getMotorOutput(1), 1300); // normal
    }
}
