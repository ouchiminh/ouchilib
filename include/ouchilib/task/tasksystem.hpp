#pragma once
#include <list>
#include <vector>
#include <chrono>
#include "task.hpp"

namespace ouchi::task{

/***
ex)
task t1{[](){}, key};
task t2{[](){}, key};
tasksystem<key> ts;
ts.register_task(t1, t2);

ts.launch(1); // thread count
***/

template<class Key>
class tasksystem {
    std::list<std::reference_wrapper<taskinfo<Key>>> tasks_;
    std::list<std::pair<std::future<void>, std::reference_wrapper<taskinfo<Key>>>>
        taskthreads_;
    std::future<void> mainthread_;

    template<class Head, class ...Task>
    void register_task_impl(Head& head, Task& ...tasks)
    {
        static_assert(std::is_base_of_v<taskinfo<Key>, Head>,
                      "task must be derived from taskinfo<T>");
        tasks_.emplace_back(head);
        register_task_impl(tasks...);
    }
    void register_task_impl(){}
public:
    using key_type = std::remove_reference_t<Key>;

    // tasksystem doesn't own each task.
    // just have reference.
    // please make sure that the tasks' lifetime is long enough.
    template<class ...Task>
    void register_task(Task& ...tasks)
    {
        register_task_impl(tasks...);
    }
    template<class It,
             std::enable_if_t<std::is_base_of_v<taskinfo<Key>,
                                                typename std::iterator_traits<It>::value_type>>*
             = nullptr>
    void register_task(It first, It last)
    {
        tasks_.insert(tasks_.end(), first, last);
    }

    // run tasks async. 0 is no limit.
    void launch(unsigned thread_count = 0)
    {
        mainthread_ = std::async(std::launch::async,
                                 [this, thread_count]() {run(thread_count); });
    }
    // launch and wait
    void run(unsigned thread_count = 0)
    {
        size_t done = 0;
        thread_count = thread_count ? thread_count : (unsigned)tasks_.size();
        for (auto& i : tasks_) {
            if (!i.get().is_ready()) continue;
            if (taskthreads_.size() < thread_count)
                taskthreads_.push_back(std::make_pair(std::async(std::launch::async,
                                                                 [&i]() {i.get()(); }),
                                                      i));
            else i.get()();
            ++done;
        }
        // poll thread status
        while(done < tasks_.size()){

            for (auto begin = taskthreads_.begin();
                 begin != taskthreads_.end(); ) {
                using namespace std::chrono_literals;
                if (begin->first.wait_for(0ms) == std::future_status::ready) {
                    done += begin->second.get().run_posttask(taskthreads_, thread_count + 1);
                    begin = taskthreads_.erase(begin);
                } else ++begin;
            }
        }
        for (auto& i : taskthreads_)
            i.first.wait();
    }
    void wait()
    {
        mainthread_.wait();
    }
};

}
