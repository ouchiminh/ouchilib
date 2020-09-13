#pragma once
#include <type_traits>
#include <list>
#include <exception>
#include <atomic>
#include <mutex>
#include "ouchilib/math/infinity.hpp"

namespace ouchi::thread {

namespace detail {

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
} // namespace detail


class task_base {
public:
    enum class status {
        waiting, working, completed, failed
    };

    task_base()
        : status_{ status::waiting }
    {}
    task_base(const task_base&) = delete;
    virtual ~task_base() = default;

    task_base& before(task_base& postprocess) {
        postprocess.preprocesses_.push_back(this);
        return *this;
    }
    task_base& after(task_base& preprocess) {
        preprocesses_.push_back(&preprocess);
        return *this;
    }
    bool ready() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        for (const task_base* p : preprocesses_) {
            if (!p->done()) return false;
        }
        return true;
    }
    bool done() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        return get_status() >= status::completed;
    }
    status get_status() const noexcept {
        return status_;
    }
protected:
    virtual bool execute() noexcept = 0;

    std::list<task_base*> preprocesses_;
    status status_;
    mutable std::recursive_mutex mtx_;
    friend class tasksystem;
};

template<class F, class = void>
class basic_task;
template<class F, class Handler, class = void>
class failable_task;

template<class F>
class basic_task<F, std::enable_if_t<std::is_invocable_v<F>>> : public task_base {
public:
    using func_type = std::remove_reference_t<F>;
    basic_task(F&& func)
        : func_{ std::forward<F>(func) }
    {}
    virtual ~basic_task() = default;
protected:
    bool get_ready() noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        if (done() || !ready()) return false;
        status_ = status::working;
        return true;
    }
    virtual bool execute() noexcept override 
    {
        if (!get_ready()) return false;
        try {
            std::invoke(func_);
            status_ = status::completed;
        } catch (...) { status_ = status::failed; }
        return true;
    }
    
    func_type func_;
};

struct retry_on_fail {
    retry_on_fail(size_t max_try)
        : max_try_{ max_try }
    {}
    retry_on_fail(ouchi::math::infinity max_try)
        : max_try_{ max_try }
    {}

    bool can_retry(size_t current)
    {
        return std::visit(detail::overloaded{
            [current](size_t m) {return current < m; },
            [current](ouchi::math::infinity m) {return current < m; }
            }, max_try_);
    }
private:
    std::variant<size_t, ouchi::math::infinity> max_try_;
};

template<class F, class Handler>
class failable_task<F, Handler, std::enable_if_t<std::is_invocable_v<F> && std::is_invocable_v<Handler, std::exception_ptr>>> : public basic_task<F> {
public:
    using error_handler_type = std::remove_reference_t<Handler>;
    failable_task(F&& func, const retry_on_fail& r, Handler&& h)
        : basic_task<F>(std::forward<F>(func))
        , r_{ r }
        , error_handler_{ std::forward<Handler>(h) }
    {}
protected:
    virtual bool execute() noexcept override
    {
        size_t retry_count = 0;
        if (!basic_task<F>::get_ready()) return false;
        do {
            try {
                std::invoke(basic_task<F>::func_);
                task_base::status_ = task_base::status::completed;
                return true;
            } catch (...) {
                std::invoke(error_handler_, std::current_exception());
            }
        } while (r_.can_retry(retry_count++));
        task_base::status_ = task_base::status::failed;
        return true;
    }
private:
    retry_on_fail r_;
    error_handler_type error_handler_;
};
}

