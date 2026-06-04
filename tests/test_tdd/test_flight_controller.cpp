#include "doctest.h"
#include "core/FlightController.h"
#include "simulation/SimulatedHardware.h"

TEST_CASE("FlightController core logic and failsafes") {
    SimulatedIMU imu;
    SimulatedPPMReceiver ppm;
    SimulatedMotors motors;
    SimulatedBatteryMonitor battery;

    FlightController fc(imu, ppm, motors, battery);
    fc.init();

    SUBCASE("Idle throttle disarm and cutoff behavior") {
        // Armed (ch4=1600) but throttle at 1000 (< 1050 cutoff) — tests throttle cutoff path
        ppm.setOverride(2, 1000);
        ppm.setOverride(4, 1600); // AUX1 armed
        ppm.setOverrideActive(true);

        fc.update(0.004f);

        // Throttle cutoff resets all motors to 1000
        CHECK_EQ(motors.getMotorOutput(0), 1000);
        CHECK_EQ(motors.getMotorOutput(1), 1000);
        CHECK_EQ(motors.getMotorOutput(2), 1000);
        CHECK_EQ(motors.getMotorOutput(3), 1000);
    }

    SUBCASE("Armed throttle and motor mixing at level flight") {
        // Throttle = 1500, others centered, AUX1 armed
        ppm.setOverride(0, 1500); // Roll
        ppm.setOverride(1, 1500); // Pitch
        ppm.setOverride(2, 1500); // Throttle
        ppm.setOverride(3, 1500); // Yaw
        ppm.setOverride(4, 1600); // AUX1 armed
        ppm.setOverrideActive(true);

        // Perfectly level IMU readings
        imu.setOverride(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        imu.setOverrideActive(true);

        fc.update(0.004f);

        // Motor mixing: MotorInput = 1.024 * (Throttle +/- Roll +/- Pitch +/- Yaw)
        // With level inputs, all error/corrections = 0.
        // Expected speed: 1.024 * 1500 = 1536
        int expectedSpeed = static_cast<int>(1.024f * 1500.0f);
        CHECK_EQ(motors.getMotorOutput(0), expectedSpeed);
        CHECK_EQ(motors.getMotorOutput(1), expectedSpeed);
        CHECK_EQ(motors.getMotorOutput(2), expectedSpeed);
        CHECK_EQ(motors.getMotorOutput(3), expectedSpeed);
    }

    SUBCASE("Failsafe triggers on PPM receiver signal loss") {
        // Drone is flying: Throttle = 1600, AUX1 armed
        ppm.setOverride(2, 1600);
        ppm.setOverride(4, 1600); // AUX1 armed
        ppm.setOverrideActive(true);
        fc.update(0.004f);
        CHECK_EQ(motors.getMotorOutput(0), static_cast<int>(1.024f * 1600.0f));

        // Signal lost
        ppm.setSignalLostOverride(true);
        fc.update(0.004f);

        // Should cutoff motors on PPM loss
        CHECK_EQ(motors.getMotorOutput(0), 1000);
        CHECK_EQ(motors.getMotorOutput(1), 1000);
    }
}
