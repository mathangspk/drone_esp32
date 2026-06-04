#include "doctest.h"
#include "core/PIDController.h"

TEST_CASE("PIDController basic calculations and constraints") {
    // Kp = 1.0, Ki = 2.0, Kd = 0.5
    PIDController pid(1.0f, 2.0f, 0.5f);

    SUBCASE("Initial values and reset behavior") {
        CHECK_EQ(pid.getError(), 0.0f);
        CHECK_EQ(pid.getIterm(), 0.0f);
        
        pid.update(10.0f, 0.0f, 0.0f, 0.004f); // run an update
        pid.reset();
        
        CHECK_EQ(pid.getError(), 0.0f);
        CHECK_EQ(pid.getIterm(), 0.0f);
    }

    SUBCASE("Proportional response only") {
        PIDController p_only(1.5f, 0.0f, 0.0f);
        float output = p_only.update(10.0f, 0.0f, 0.0f, 0.004f);
        // Output = Pterm = 1.5 * 10 = 15
        CHECK_EQ(output, 15.0f);
    }

    SUBCASE("Integral summation and windup limits") {
        PIDController i_only(0.0f, 10.0f, 0.0f);
        
        // Step 1: error = 10, prevError = 0. Iterm = 0 + 10 * (10 + 0) * 0.004 / 2 = 0.2
        float output = i_only.update(10.0f, 0.0f, 0.0f, 0.004f);
        CHECK_EQ(output, doctest::Approx(0.2f));
        CHECK_EQ(i_only.getIterm(), doctest::Approx(0.2f));

        // Let's test anti-windup clamping (Iterm limit: +/- 400)
        PIDController windup(0.0f, 50000.0f, 0.0f);
        // Error = 100, dt = 0.1s. Iterm = 0 + 50000 * 100 * 0.1 = 500,000 -> clamps to 400
        float capped_out = windup.update(100.0f, 0.0f, 0.0f, 0.1f);
        CHECK_EQ(capped_out, 400.0f);
        CHECK_EQ(windup.getIterm(), 400.0f);
    }

    SUBCASE("Derivative response") {
        PIDController d_only(0.0f, 0.0f, 0.2f);
        // Dterm = D * (error - prevError) / dt
        // error = 10, prevError = 5, dt = 0.004
        // Output = 0.2 * (10 - 5) / 0.004 = 0.2 * 1250 = 250
        float output = d_only.update(10.0f, 5.0f, 0.0f, 0.004f);
        CHECK_EQ(output, doctest::Approx(250.0f));
    }

    SUBCASE("Overall output clamping") {
        PIDController high_gain(1000.0f, 0.0f, 0.0f);
        // output = 1000 * 5 = 5000 -> clamps to 400
        float output = high_gain.update(5.0f, 0.0f, 0.0f, 0.004f);
        CHECK_EQ(output, 400.0f);

        float neg_output = high_gain.update(-5.0f, 0.0f, 0.0f, 0.004f);
        CHECK_EQ(neg_output, -400.0f);
    }

    SUBCASE("D-on-measurement: no derivative kick on setpoint change") {
        // kd=0.2, alpha=1.0 (no LPF). Prime prevMeasurement to 5 with first call.
        PIDController d_meas(0.0f, 0.0f, 0.2f, 1.0f);
        d_meas.update(0.0f, 5.0f, 0.004f); // prevMeasurement_ is now 5

        // Setpoint jumps (error = 10) but measurement stays at 5 → D term must be zero
        float out = d_meas.update(10.0f, 5.0f, 0.004f);
        CHECK_EQ(out, 0.0f);
    }

    SUBCASE("D-term LPF decays filtered state") {
        // kd=0.01, alpha=0.5. First call: dRaw=-0.01*(5-0)/0.004=-12.5, filtered=-6.25
        PIDController d_lpf(0.0f, 0.0f, 0.01f, 0.5f);
        float out1 = d_lpf.update(0.0f, 5.0f, 0.004f);
        CHECK_EQ(out1, doctest::Approx(-6.25f));

        // Second call: measurement unchanged → dRaw=0, filtered=0.5*(-6.25)=-3.125
        float out2 = d_lpf.update(0.0f, 5.0f, 0.004f);
        CHECK_EQ(out2, doctest::Approx(-3.125f));
    }
}
