﻿#include "ouchilib/thread/task.hpp"
#include "ouchilib/thread/tasksystem.hpp"
#include "../test.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <chrono>
#include <atomic>

DEFINE_TEST(test_task_instantiate)
{
    using namespace ouchi::task;
    using namespace std::string_literals;
    task t1{ []() {return 0; }, "task1"s };
    task t2{ []() {return 1; }, "task2"s };
    auto_arg_task t3(+[](int i, int j) {assert(i == 0 && j == 1); return 12; },
                     "task3"s, std::make_tuple("task1"s, "task2"s));
    auto_arg_task t4(+[](int b) {assert(b == 12); },
                     "task4"s, std::make_tuple("task3"s));
    task t5{ []() {}, "task5"s };

    t1 --> t3 --> t5;
    t2 --> t3 --> t4;
    // because there is no task control system, 
    // each root task should be called manually.
    t1();
    t2();
}

namespace {
std::condition_variable cv;
std::mutex m;
}

DEFINE_TEST(test_tasksystem)
{
    using namespace ouchi::task;
    using namespace std::string_literals;
    using namespace std::chrono_literals;
    bool done = false;
    task t1{ []() {/*std::unique_lock<std::mutex> l(m); cv.wait(l);*/ return 0; }, "task1"s };
    task t2{ []() {/*std::unique_lock<std::mutex> l(m); cv.wait(l);*/ return 1; }, "task2"s };
    auto_arg_task t3(+[](int i, int j) {std::this_thread::sleep_for(70ms); (i == 0 && j == 1); return 12; },
                     "task3"s, std::make_tuple("task1"s, "task2"s));
    auto_arg_task<std::string, std::function<void(int)>, int> t4([&done](int b) { (b == 12); done = true; },
                                                                 "task4"s, std::make_tuple("task3"s));

    tasksystem<std::string> ts;
    ts.register_task(t1, t2, t3, t4);
    t1 --> t3;
    t2 --> t3 --> t4;

    ts.run();
    CHECK_EQUAL(done, true);
}

DEFINE_TEST(test_tasksystem_threadlimit)
{
    using namespace ouchi::task;
    using namespace std::string_literals;
    std::atomic_bool done = false;
    task t1{ []() {return 0; }, "task1"s };
    task t2{ []() {return 1; }, "task2"s };
    auto_arg_task t3(+[](int i, int j) {assert(i == 0 && j == 1); return 12; },
                     "task3"s, std::make_tuple("task1"s, "task2"s));
    auto_arg_task<std::string, std::function<void(int)>, int> t4([&done](int b) {assert(b == 12); done = true; },
                                                                 "task4"s, std::make_tuple("task3"s));

    tasksystem<std::string> ts;
    ts.register_task(t1, t2, t3, t4);
    t1 --> t3;
    t2 --> t3 --> t4;
    ts.run(1);
    CHECK_EQUAL(done, true);
}

namespace {
uintmax_t fibonacci(uintmax_t n)
{
    return n < 2
        ? n
        : fibonacci(n - 1) + fibonacci(n - 2);
}
}

DEFINE_TEST(test_tasksystem_fibonacci)
{
    using namespace ouchi::task;
    using namespace std::literals;
    task t10([]() {return fibonacci(10); }, "t10"sv);
    task t11([]() {return fibonacci(11); }, "t11"sv);
    auto_arg_task t12(+[](uintmax_t n1, uintmax_t n2) {assert(n1 + n2 == fibonacci(12)); },
                      "t42"sv, std::make_tuple("t40"sv, "t41"sv));
    t12 <-- t10;
    t12 <-- t11;
    tasksystem<std::string_view> ts;
    ts.register_task(t10, t11, t12);
    ts.run();
    return;
}
