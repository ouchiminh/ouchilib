#pragma once
#include <iostream>
#include "../test.hpp"
#include "ouchilib/task/tasksystem.hpp"

namespace ouchi::test {

class counterasetask : public task::task {
    void run()
    {
        std::cout << "success" << std::endl;
    }
public:
    counterasetask()
        : task(ouchi::task::task_function(this, &counterasetask::run),
               ouchi::task::erase_condition(1ull))
    {}
};

class timeerasetask : public task::task {
    void run()
    {
        std::cout << "success2" << std::endl;
    }
public:
    timeerasetask()
        : task(ouchi::task::task_function(this, &timeerasetask::run),
               ouchi::task::erase_condition(std::chrono::milliseconds(50)))
    {}
};
class nevererasetask : public task::task {
    void run()
    {
        std::cout << "never erase" << std::endl;
    }
public:
    nevererasetask()
        : task(ouchi::task::task_function(this, &nevererasetask::run))
    {}
};

DEFINE_TEST(test_tasksystem)
{
    using namespace std::literals::chrono_literals;
    task::tasksystem ts;
    ts.create<counterasetask>();
    ts.create<timeerasetask>();
    ts.create<nevererasetask>();
    ts.update();
    std::this_thread::sleep_for(60ms);
    ts.update();
    ts.update();
}

}
