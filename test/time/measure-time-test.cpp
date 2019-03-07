#pragma once
#include <thread>
#include "../test.hpp"
#include "ouchilib/utl/time-measure.hpp"

namespace ouchi::test {

DEFINE_TEST(test_measure_time) {
    namespace chrono = std::chrono;
    using namespace std::literals;
    std::chrono::high_resolution_clock::duration d(0);
    {
        ouchi::time_measure tm(d);
        std::this_thread::sleep_for(chrono::milliseconds(500));
    }
    REQUIRE_TRUE(d - 450ms <= 100ms);
    LOG(std::to_string(chrono::duration_cast<chrono::milliseconds>(d).count()));

    d = 0s;
    {
        ouchi::time_measure tm(d);
        std::this_thread::sleep_for(chrono::milliseconds(500));
        tm.stop();

        std::this_thread::sleep_for(chrono::milliseconds(100));

        tm.restart();
        std::this_thread::sleep_for(chrono::milliseconds(100));
    }
    REQUIRE_TRUE(d - 550ms <= 100ms);
    LOG(std::to_string(chrono::duration_cast<chrono::milliseconds>(d).count()));
}

}
