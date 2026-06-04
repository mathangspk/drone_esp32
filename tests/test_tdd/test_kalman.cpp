#include "doctest.h"
#include "core/KalmanFilter.h"

TEST_CASE("KalmanFilter updates and convergence") {
    // Standard initialization: state = 0, uncertainty = 4 (from 2*2)
    KalmanFilter kf(0.0f, 4.0f);

    SUBCASE("Initial states") {
        CHECK_EQ(kf.getState(), 0.0f);
        CHECK_EQ(kf.getUncertainty(), 4.0f);
    }

    SUBCASE("Single step update matching Instructables math") {
        // dt = 0.004, gyro_rate = 10.0, acc_angle = 5.0
        // State prior = 0 + 0.004 * 10 = 0.04
        // Uncertainty prior = 4 + 0.004 * 0.004 * 16 = 4.000256
        // Gain = 4.000256 / (4.000256 + 9) = 4.000256 / 13.000256 = 0.307706
        // State posterior = 0.04 + 0.307706 * (5.0 - 0.04) = 0.04 + 0.307706 * 4.96 = 1.5662
        // Uncertainty posterior = (1 - 0.307706) * 4.000256 = 0.692294 * 4.000256 = 2.76935
        kf.update(10.0f, 5.0f, 0.004f);

        CHECK_EQ(kf.getState(), doctest::Approx(1.5662f).epsilon(0.001));
        CHECK_EQ(kf.getUncertainty(), doctest::Approx(2.76935f).epsilon(0.001));
    }

    SUBCASE("Convergence on steady measurement") {
        // Run multiple updates with a constant angle measurement of 15.0 degrees
        // and 0 gyro rate. The state should converge toward 15.0.
        for (int i = 0; i < 50; ++i) {
            kf.update(0.0f, 15.0f, 0.004f);
        }
        CHECK_EQ(kf.getState(), doctest::Approx(15.0f).epsilon(0.05));
        // The uncertainty should have decreased significantly
        CHECK_LT(kf.getUncertainty(), 1.0f);
    }
}
