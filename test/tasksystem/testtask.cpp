#include "ouchilib/thread/tasksystem.hpp"
#include "../test.hpp"
#include "ouchilib/result/result.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <chrono>
#include <atomic>
#include <sstream>
#include <chrono>

using namespace ouchi::thread;

DEFINE_TEST(task_simple_test)
{
    std::stringstream sn;
    std::stringstream sa;
    tasksystem ts;
    ts.create_task([&sn]() {sn << 0; }).before(ts.create_task([&sn]() {sn << 1; }));
    ts.create_task([&sa]() {sa << 'b'; }).after(ts.create_task([&sa]() {sa << 'a'; }));
    ts.launch();
    // 01ab
    CHECK_EQUAL(sn.str(), "01");
    CHECK_EQUAL(sa.str(), "ab");
}
DEFINE_TEST(task_error_handle_test)
{
    tasksystem ts;
    int c = 0;
    ts.create_task([]() { throw 1; },
                   retry_on_fail{10},
                   [&c](const auto& eptr) {
                       try {
                           std::rethrow_exception(eptr);
                       } catch (int i) { c += i; }
                   });
    ts.launch();
    CHECK_EQUAL(c, 11);
}

DEFINE_TEST(task_paralel_test)
{
    namespace chrono = std::chrono;
    using namespace std::literals;
    std::stringstream sn;
    std::stringstream sa;
    tasksystem ts;
    ts.create_task([&sn]() {sn << 0; }).before(ts.create_task([&sn]() {sn << 1;}));
    ts.create_task([&sa]() {sa << 'b'; }).after(ts.create_task([&sa]() {sa << 'a';}));
    ts.launch(4);
    // 01ab
    CHECK_EQUAL(sn.str(), "01");
    CHECK_EQUAL(sa.str(), "ab");
}

