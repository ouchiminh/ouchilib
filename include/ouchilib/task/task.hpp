#pragma once
#include "erase_condition.hpp"

namespace ouchi::task {

class task_function {
    std::function<void()> f_;
public:
    explicit task_function(std::function<void()>&& f)
        : f_{ std::move(f) }
    {}
    explicit task_function(const std::function<void()>& f)
        : f_{ f }
    {}
    template<class Class>
    task_function(Class* ptr, void(Class::*fp)())
        : f_{std::bind(fp, ptr)}
    {}
    void operator()() const
    {
        f_();
    }
};

template<class Class>
[[nodiscard]] [[deprecated("please use task_function::task_function(Class*, void(Class::*)())")]]
task_function convert_method_to_task_function(Class* ptr, void(Class::* fval)())
{
    return task_function{ std::bind(fval, ptr) };
}

class task {
    task_function f_;
    erase_condition cond_;
public:
    task(const task_function& tf, const erase_condition& ec = erase_condition{})
        : f_{ tf }
        , cond_{ ec }
    {}
    task(task_function&& tf, erase_condition&& ec = erase_condition{})
        : f_{ std::move(tf) }
        , cond_{ std::move(ec) }
    {}
    ~task() = default;

    // falseなら死ぬ
    bool operator()()
    {
        return cond_.update_cond()
            ? false
            : (f_(), true);
    }
    [[nodiscard]]
    bool is_destroyable() const
    {
        return static_cast<bool>(cond_);
    }
protected:
    void set_function(const task_function& f)
    {
        f_ = f;
    }
    void set_function(task_function&& f)
    {
        f_ = std::move(f);
    }
    void set_erase_condition(const erase_condition& c)
    {
        cond_ = c;
    }
    void set_erase_condition(erase_condition&& c)
    {
        cond_ = std::move(c);
    }
    void suicide()
    {
        cond_ = erase_condition(std::bool_constant<true>{});
    }
};

}
