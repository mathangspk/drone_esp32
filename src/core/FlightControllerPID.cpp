#include "core/FlightController.h"

#ifndef NATIVE_BUILD
#include <Preferences.h>
#endif

void FlightController::loadPIDGains() {
    float r_kp=kDefaultRateKp,  r_ki=kDefaultRateKi,  r_kd=kDefaultRateKd;
    float p_kp=kDefaultRateKp,  p_ki=kDefaultRateKi,  p_kd=kDefaultRateKd;
    float y_kp=kDefaultYawKp,   y_ki=kDefaultYawKi,   y_kd=kDefaultYawKd;
    float ra_kp=kDefaultAngleKp, ra_kd=kDefaultAngleKd;
    float pa_kp=kDefaultAngleKp, pa_kd=kDefaultAngleKd;

#ifndef NATIVE_BUILD
    Preferences prefs;
    prefs.begin("pid", true);
    r_kp = prefs.getFloat("r_kp", r_kp);
    r_ki = prefs.getFloat("r_ki", r_ki);
    r_kd = prefs.getFloat("r_kd", r_kd);
    p_kp = prefs.getFloat("p_kp", p_kp);
    p_ki = prefs.getFloat("p_ki", p_ki);
    p_kd = prefs.getFloat("p_kd", p_kd);
    y_kp = prefs.getFloat("y_kp", y_kp);
    y_ki = prefs.getFloat("y_ki", y_ki);
    y_kd = prefs.getFloat("y_kd", y_kd);
    ra_kp = prefs.getFloat("ra_kp", ra_kp);
    ra_kd = prefs.getFloat("ra_kd", ra_kd);
    pa_kp = prefs.getFloat("pa_kp", pa_kp);
    pa_kd = prefs.getFloat("pa_kd", pa_kd);
    prefs.end();
#endif

    rollRatePid_.setGains(r_kp, r_ki, r_kd);
    pitchRatePid_.setGains(p_kp, p_ki, p_kd);
    yawRatePid_.setGains(y_kp, y_ki, y_kd);
    rollAnglePid_.setGains(ra_kp, 0.0f, ra_kd);
    pitchAnglePid_.setGains(pa_kp, 0.0f, pa_kd);
}
