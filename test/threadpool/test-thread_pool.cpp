#include "../test.hpp"
#include "ouchilib/thread/thread-pool.hpp"
#include "ouchilib/utl/time-measure.hpp"

DEFINE_TEST(test_thread_pool)
{
    constexpr unsigned c = 32;
    size_t l = 1;
    while (l-->0) {
        using namespace std::chrono_literals;
        bool flag[c] = {};
        ouchi::thread::thread_pool tp(8);
        for (auto i = 0; i < c; ++i) {
            tp.push([i, &flag]() { flag[i] = true; });
        }
        tp.wait();
        //std::this_thread::sleep_for(50ms);
        for (auto i = 0; i < c; ++i) {
            //std::cerr << i << ' ' << flag[i] << '\n';
            //if (!flag[i]) std::cerr << l;
            CHECK_TRUE(flag[i]);
        }
    }
}
