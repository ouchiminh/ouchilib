#include "../test.hpp"
#include "ouchilib/thread/thread-pool.hpp"

DEFINE_TEST(test_thread_pool)
{
    constexpr unsigned c = 36;
    bool flag[c] = {};
    ouchi::thread::thread_pool tp(8);
    for (auto i = 0; i < c; ++i) {
        tp.push([i, &flag]() { flag[i] = true; });
    }
    tp.wait();
    for (auto i = 0; i < c; ++i) {
        //std::cerr << i << ' ' << flag[i] << '\n';
        CHECK_TRUE(flag[i]);
    }
}
