#pragma once
#include <type_traits>

namespace ouchi::thread {

struct work_base {
    virtual void operator()() = 0;
};

namespace detail {

template<class F>
class work : public work_base {
    static_assert(std::is_invocable_v<F>);

    F func_;
public:
    work(F&& func)
        : func_{ std::forward<F>(func) }
    {}
    template<class ...Args>
    work(Args&& ...args)
        : func_(std::forward<Args>(args)...)
    {}
    void operator()() override { func_(); }
};

} // namespace detail

}
