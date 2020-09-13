#pragma once
#include <type_traits>
#include <functional>
#include <variant>
#include <compare>
#include <memory>
#include <future>
#include <list>
#include <map>
#include <cstdint>
#include "task.hpp"

namespace ouchi::thread {

class tasksystem {
public:
    template<class F>
    auto create_task(F&& func)
        -> std::enable_if_t<std::is_invocable_v<std::remove_cvref_t<F>>, task_base&>
    {
        return *tasks_.emplace_back(new basic_task<F>(std::forward<F>(func)));
    }

    template<class F>
    auto create_task(F&& func, const retry_on_fail& r)
        -> std::enable_if_t<std::is_invocable_v<std::remove_cvref_t<F>>, task_base&>
    {
        return create_task(std::forward<F>(func), r, [](...) {});
    }

    template<class F, class ErrorHandler>
    auto create_task(F&& func, const retry_on_fail& r, ErrorHandler&& error_handler)
        -> std::enable_if_t<std::is_invocable_v<std::remove_cvref_t<F>>, task_base&>
    {
        return *tasks_.emplace_back(new failable_task<F, ErrorHandler>(std::forward<F>(func), r, std::forward<ErrorHandler>(error_handler)));
    }

    void launch(size_t max_thread_count = 0) {
        size_t completed = 0;
        std::map<std::intptr_t, std::future<void>> results;
        while (completed != tasks_.size()) {
            completed = 0;
            for (auto& taskptr : tasks_) {
                if (taskptr->done()) {
                    ++completed;
                    continue;
                }
                if (taskptr->get_status() == task_base::status::working) {
                    using namespace std::literals::chrono_literals;
                    auto& f = results[(std::intptr_t)taskptr.get()];
                    if (std::future_status::ready == f.wait_for(1us)) {
                        (void)f.get();
                        results.erase((std::intptr_t)taskptr.get());
                    }
                }
                if (taskptr->ready() && results.size() < max_thread_count) {
                    results.emplace(std::piecewise_construct,
                                    std::forward_as_tuple((std::intptr_t)taskptr.get()),
                                    std::forward_as_tuple(std::async(std::launch::async, [&taskptr]() { (void)taskptr->execute(); })));
                }
                else if (taskptr->ready()) {
                    (void)taskptr->execute();
                }
            }
        }
    }

private:
    std::list<std::unique_ptr<task_base>> tasks_;
};

}

