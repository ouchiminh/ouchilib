#pragma once
#include <stdexcept>

namespace ouchi::task {

struct task_not_ready : std::runtime_error{
    task_not_ready(std::string_view msg)
        : std::runtime_error(msg.data())
    {}
    task_not_ready()
        : std::runtime_error("task dependency is not solved")
    {}
};

}
