#pragma once
#include <type_traits>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <vector>
#include <atomic>
#include <algorithm>

#include "work.hpp"

namespace ouchi::thread {

class thread_pool {
    std::mutex work_mtx_;

    std::queue<std::unique_ptr<work_base>> works_;
    std::vector<std::thread> threads_;
    std::condition_variable cv_;
    volatile std::atomic_bool pause_ = false, finish_ = false;
    std::atomic_uint processing_ = 0;

public:
    thread_pool(size_t thread_count = 2)
    {
        auto poll = [this]() {
            std::unique_ptr<work_base> f;
            while (true) {
                {
                    std::unique_lock<decltype(work_mtx_)> ul(work_mtx_);
                    while (works_.empty() || pause_) {
                        if (finish_) return;
                        cv_.wait(ul);
                    }
                    ++processing_;
                    f = std::move(works_.front());
                    works_.pop();
                }
                f->operator()();
                --processing_;
                f.reset();
            }
        };
        auto actual_tc = std::max(thread_count, 1ull);
        threads_.reserve(actual_tc);
        for (size_t i = 0; i < actual_tc; ++i)
            threads_.emplace_back(poll);
    }
    thread_pool(const thread_pool&) = delete;
    ~thread_pool()
    {
        {
            std::unique_lock<decltype(work_mtx_)> ul(work_mtx_);
            std::queue<std::unique_ptr<work_base>> tmp;
            works_.swap(tmp);
        }
        wait();
        finish_ = true;
        for (auto& i : threads_) {
            cv_.notify_all();
            i.join();
        }
    }

    template<class F, class ...Args>
    void emplace(Args&& ...args)
    {
        {
            std::unique_lock<decltype(work_mtx_)> ul(work_mtx_);
            static_assert(std::is_constructible_v<F, Args...>);
            if constexpr (std::is_base_of_v<work_base, F>)
                works_.push(std::make_unique<F>(std::forward<Args>(args)...));
            else
                works_.push(std::make_unique<detail::work<F>>(std::forward<Args>(args))...);
        }
        cv_.notify_all();
    }

    template<class F>
    void push(F&& functor)
    {
        emplace<F>(std::forward<F>(functor));
    }

    // unprocessed work
    size_t remaining() const noexcept
    {
        return processing_ + works_.size();
    }

    // number of threads
    size_t size() const noexcept {
        return threads_.size();
    }

    // stop after current work
    void pause() noexcept {
        pause_ = true;
    }

    void wait() noexcept
    {
        while (remaining())
            std::this_thread::yield();
    }

    void resume() noexcept
    {
        pause_ = false;
        cv_.notify_all();
    }
};

}
