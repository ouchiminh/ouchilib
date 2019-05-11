#include "ouchilib/task/task.hpp"
#include "../test.hpp"
#include <iostream>
#include <string>
#include <tuple>

void task3(int i, int j)
{
    std::cout << i << j; 
}
DEFINE_TEST(test_task_instantiate)
{
    using namespace ouchi::task;
    using namespace std::string_literals;
    task t1{ []() {return 0; }, "task1"s };
    task t2{ []() {return 1; }, "task2"s };
    auto_arg_task t3(+[](int i, int j) {assert(i == 0 && j == 1); }, "task3"s, std::make_tuple("task1"s, "task2"s));
    t1 --> t3;
    t2 --> t3;
    // because there is no task control system, 
    // each root task should be called manually.
    t1();
    t2();
}
