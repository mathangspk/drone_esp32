#include "core/FlightController.h"

#ifndef NATIVE_BUILD
#include <Preferences.h>
#endif

void FlightController::loadPIDGains() {
    float r_kp=0.7f, r_ki=0.0f, r_kd=0.01f, p_kp=0.7f, p_ki=0.0f, p_kd=0.01f;
    float y_kp=2.0f, y_ki=12.0f, y_kd=0.0f;
    float ra_kp=1.5f, ra_kd=0.0f, pa_kp=1.5f, pa_kd=0.0f;

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
