#ifndef SIMULATEDHARDWARE_H
#define SIMULATEDHARDWARE_H

#include "interfaces/IIMU.h"
#include "interfaces/IPPM.h"
#include "interfaces/IMotors.h"
#include "interfaces/IBattery.h"

class SimulatedIMU : public IIMU {
public:
    void readSensor() override {}
    void getGyroRates(float& r, float& p, float& y) const override {
        r = active_ ? oRollRate_ : 0.0f;
        p = active_ ? oPitchRate_ : 0.0f;
        y = active_ ? oYawRate_ : 0.0f;
    }
    void getAccAngles(float& r, float& p) const override {
        r = active_ ? oRollAngle_ : 0.0f;
        p = active_ ? oPitchAngle_ : 0.0f;
    }
    void setOverride(float rRate, float pRate, float yRate, float rAngle, float pAngle) override {
        oRollRate_ = rRate; oPitchRate_ = pRate; oYawRate_ = yRate;
        oRollAngle_ = rAngle; oPitchAngle_ = pAngle;
    }
    void setOverrideActive(bool active) override { active_ = active; }
    bool isOverrideActive() const override { return active_; }
private:
    bool active_ = false;
    float oRollRate_ = 0.0f, oPitchRate_ = 0.0f, oYawRate_ = 0.0f;
    float oRollAngle_ = 0.0f, oPitchAngle_ = 0.0f;
};

class SimulatedPPMReceiver : public IPPM {
public:
    void readChannels() override {}
    int getChannel(int idx) const override {
        if (active_ && idx >= 0 && idx < 6) return oChannels_[idx];
        return (idx == 2) ? 1000 : 1500; // Idle throttle, others centered
    }
    bool isSignalLost() const override { return signalLost_; }
    void setOverride(int idx, int val) override { if (idx >= 0 && idx < 6) oChannels_[idx] = val; }
    void setSignalLostOverride(bool lost) override { signalLost_ = lost; }
    void setOverrideActive(bool active) override { active_ = active; }
    bool isOverrideActive() const override { return active_; }
private:
    bool active_ = false;
    bool signalLost_ = false;
    int oChannels_[6] = {1500, 1500, 1000, 1500, 1500, 1500};
};

class SimulatedMotors : public IMotors {
public:
    void writeMotors(int m1, int m2, int m3, int m4) override {
        m_[0] = m1; m_[1] = m2; m_[2] = m3; m_[3] = m4;
    }
    void setOverride(int idx, int val, bool act) override {
        if (idx >= 0 && idx < 4) { oActive_[idx] = act; oVal_[idx] = val; }
    }
    int getMotorOutput(int idx) const override {
        if (idx >= 0 && idx < 4) return oActive_[idx] ? oVal_[idx] : m_[idx];
        return 1000;
    }
    bool isMotorOverridden(int idx) const override {
        return (idx >= 0 && idx < 4) ? oActive_[idx] : false;
    }
private:
    int m_[4] = {1000, 1000, 1000, 1000};
    int oVal_[4] = {1000, 1000, 1000, 1000};
    bool oActive_[4] = {false, false, false, false};
};

class SimulatedBatteryMonitor : public IBattery {
public:
    float readVoltage() const override { return active_ ? oVoltage_ : 11.1f; }
    bool isLow() const override { return readVoltage() < LOW_VOLTAGE_THRESHOLD; }
    void setOverride(float v) override { oVoltage_ = v; }
    void setOverrideActive(bool active) override { active_ = active; }
    bool isOverrideActive() const override { return active_; }
private:
    bool active_ = false;
    float oVoltage_ = 11.1f;
};

#endif // SIMULATEDHARDWARE_H
