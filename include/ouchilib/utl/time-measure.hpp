#pragma once
#include <chrono>

namespace ouchi {

template<class Clock = std::chrono::high_resolution_clock,
         class Duration = typename Clock::duration>
class time_measure {
    using TimePoint = decltype(Clock::now());
    Duration* const duration_;
    TimePoint lap_;
    const bool delete_flag_;
    bool working_;
public:
    time_measure(Duration& duration)
        : duration_(&duration)
        , lap_(Clock::now())
        , delete_flag_(false)
        , working_(true)
    {}

    time_measure()
        : duration_(new Duration())
        , lap_(Clock::now())
        , delete_flag_(true)
        , working_(true)
    {
        reset();
    }
    
    ~time_measure() {
        stop();
        if (delete_flag_)
            delete duration_;
    }

    void reset() noexcept {
        *duration_ = Duration(0);
        lap_ = Clock::now();
        working_ = true;
    }
    void restart() noexcept {
        auto n = Clock::now();
        lap_ = n;
        working_ = true;
    }
    void stop() noexcept {
        if (!working_) return;
        auto n = Clock::now();
        *duration_ += n - lap_;
        lap_ = n;
        working_ = false;
    }
    Duration get() const noexcept {
        return *duration_;
    }
    template<class D>
    D get() const noexcept
    {
        return std::chrono::duration_cast<D>(*duration_);
    }


};

template<class Clock = std::chrono::high_resolution_clock,
         class Duration = typename Clock::duration,
         class F,
         class ...Args>
[[nodiscard]]
Duration measure(F&& func, Args&& ...args)
{
    auto begin = Clock::now();
    func(std::forward<Args>(args)...);
    return std::chrono::duration_cast<Duration>(Clock::now() - begin);
}
}