#ifndef DRONE_PID_CONTROLLER_H
#define DRONE_PID_CONTROLLER_H

#include "PIDController/PIDController.h"

class DronePIDController {
private:
    // PID controllers for each axis
    PIDController pidRoll;
    PIDController pidPitch;
    PIDController pidYaw;

    // PID output limits
    float maxOutput;
    
    // Angle limits
    float maxAngle;
    
    // Current PID outputs
    float rollOutput;
    float pitchOutput;
    float yawOutput;

public:
    DronePIDController(
        // Roll PID gains
        float kp_roll, float ki_roll, float kd_roll,
        // Pitch PID gains
        float kp_pitch, float ki_pitch, float kd_pitch,
        // Yaw PID gains
        float kp_yaw, float ki_yaw, float kd_yaw,
        // Output and angle limits
        float max_output = 100.0f,
        float max_angle = 30.0f
    ) : 
        pidRoll(kp_roll, ki_roll, kd_roll, -max_output, max_output),
        pidPitch(kp_pitch, ki_pitch, kd_pitch, -max_output, max_output),
        pidYaw(kp_yaw, ki_yaw, kd_yaw, -max_output, max_output),
        maxOutput(max_output),
        maxAngle(max_angle),
        rollOutput(0),
        pitchOutput(0),
        yawOutput(0)
    {}

    // Update PID controllers with new sensor data
    void update(
        float rollSetpoint, float pitchSetpoint, float yawSetpoint,
        float rollActual, float pitchActual, float yawActual,
        float dt
    ) {
        // Constrain setpoints to max angle
        rollSetpoint = constrain(rollSetpoint, -maxAngle, maxAngle);
        pitchSetpoint = constrain(pitchSetpoint, -maxAngle, maxAngle);
        yawSetpoint = constrain(yawSetpoint, -maxAngle, maxAngle);

        // Compute PID outputs
        rollOutput = pidRoll.compute(rollSetpoint, rollActual, dt);
        pitchOutput = pidPitch.compute(pitchSetpoint, pitchActual, dt);
        yawOutput = pidYaw.compute(yawSetpoint, yawActual, dt);
    }

    // Get current PID outputs
    float getRollOutput() const { return rollOutput; }
    float getPitchOutput() const { return pitchOutput; }
    float getYawOutput() const { return yawOutput; }

    // Reset all PID controllers
    void reset() {
        pidRoll.reset();
        pidPitch.reset();
        pidYaw.reset();
        rollOutput = pitchOutput = yawOutput = 0;
    }

    // Set new PID gains
    void setRollGains(float kp, float ki, float kd) {
        pidRoll.setTunings(kp, ki, kd);
    }

    void setPitchGains(float kp, float ki, float kd) {
        pidPitch.setTunings(kp, ki, kd);
    }

    void setYawGains(float kp, float ki, float kd) {
        pidYaw.setTunings(kp, ki, kd);
    }
};

#endif // DRONE_PID_CONTROLLER_H 