#pragma once
#include <cassert>
#include <any>
#include <optional>
#include <map>
#include <vector>
#include <functional>
#include <utility>
#include <thread>
#include <mutex>
#include <atomic>
#include <tuple>

namespace ouchi::task {

namespace detail {

template<class T, unsigned N>
struct ntuple {
    using type = decltype(std::tuple_cat(std::declval<typename ntuple<T, N - 1>::type>(), std::declval<std::tuple<T>>()));
};
template<class T>
struct ntuple<T, 0> {
    using type = std::tuple<>;
};

template<class T, unsigned N>
using ntuple_t = typename ntuple<T, N>::type;

static_assert(std::is_same_v<std::tuple<int, int>, ntuple_t<int, 2>>);
static_assert(std::is_same_v<std::tuple<>, ntuple_t<int, 0>>);

template<class F, class Tuple, size_t ...I>
auto call_with_tuple_impl(F&& f, Tuple&& tuple, std::integer_sequence<size_t, I...>)
{
    return f(std::get<I>(tuple)...);
}
template<class F, class ...Args>
auto call_with_tuple(F&& f, const std::tuple<Args...>& args)
{
    return call_with_tuple_impl(f, args, std::make_integer_sequence<size_t, sizeof...(Args)>{});
}
}

// ex)
// task pre, post;
// pre --> post; // dependency setting
// post <-- pre; // dependency setting

template<class Key>
class taskinfo {
public:
    using key_type = std::remove_reference_t<Key>;
    struct task_dependency;
    friend task_dependency;
private:

    // this task will be lauched after all dependent task was finished.
    //       dependent | finished? | return value (if exists)
    std::map<key_type, std::pair<bool, std::any>> pretask_;
    std::vector<std::reference_wrapper<taskinfo>> posttask_;
    key_type key_;
    mutable std::mutex callguard_;
    mutable std::mutex member_rw_guard_;

    void add_pretask(const Key& key)
    {
        pretask_.insert_or_assign(key,
                                  std::make_pair<bool, std::optional<std::any>>(false, std::nullopt));
    }
    void add_posttask(taskinfo& ti) { posttask_.push_back(std::ref(ti)); }
    void set_result(const key_type& key, std::any result)
    {
        std::lock_guard<decltype(member_rw_guard_)> l(member_rw_guard_);
        this->pretask_.at(key).first = true;
        this->pretask_.at(key).second = result;
    }
protected:
    template<class T>
    T get_pretask_result(const key_type& key) const
    {
        std::lock_guard<decltype(member_rw_guard_)> l(member_rw_guard_);
        return std::any_cast<T>(pretask_.at(key).second);
    }
    void tell_result(std::any result)
    {
        for (auto& i : posttask_) {
            i.get().set_result(key_, result);
            i.get()();
        }
    }
    virtual void run() = 0;
public:
    taskinfo(key_type&& key)
        : key_(std::move(key))
    {}
    taskinfo(const key_type& key)
        : key_(key)
    {}
    virtual ~taskinfo() = default;

    const key_type& key() { return key_; }

    virtual void operator()() final
    {
        std::lock_guard<decltype(callguard_)> l(callguard_);
        for (const auto& i : pretask_)
            if (!i.second.first) return;
        run();
    }

    struct task_dependency{
        taskinfo& t;
        task_dependency(taskinfo& ti)
            : t(ti)
        {}
        task_dependency(const task_dependency&) = delete;
        task_dependency(const task_dependency&& td)
            : t(td.t)
        {}
        taskinfo& operator<(task_dependency&& t2)
        {
            (*this).t.add_pretask(t2.t.key());
            t2.t.add_posttask((*this).t);
            return t2.t;
        }
        taskinfo& operator>(task_dependency&& t2)
        {
            t2.t.add_pretask((*this).t.key());
            (*this).t.add_posttask(t2.t);
            return t2.t;
        }
    };

    task_dependency operator--() { return *this; }
    task_dependency operator--(int) { return *this; }
};

template<class Key, class F>
class task : public taskinfo<Key> {
    F f_;
public:
    using key_type = typename taskinfo<Key>::key_type;
    using taskinfo<Key>::operator--;

    task(F f, const key_type& key)
        : taskinfo<Key>(key)
        , f_(std::move(f))
    {}
    task(const key_type& key)
        : taskinfo<Key>(key)
    {}
    task(key_type&& key)
        : taskinfo<Key>(std::move(key))
    {}

protected:
    virtual void run() override
    {
        if constexpr(std::is_invocable_v<F>)
            exec();
    }

    template<class ...Args>
    void exec(Args&& ...args)
    {
        static_assert(std::is_invocable_v<F, Args...>);

        if constexpr (std::is_same_v<void, decltype(std::declval<F>()(std::declval<Args>()...))>)
        {
            f_(std::forward<Args>(args)...);
            taskinfo<Key>::tell_result(std::any{});
            return;
        } else {
            auto result = f_(std::forward<Args>(args)...);
            taskinfo<Key>::tell_result(std::any{ result });
            return;
        }
    }
};

template<class Key, class F, class ...Args>
task(F f, Key key)->task<std::remove_reference_t<Key>, F>;

template<class Key, class F, class ...Args>
class auto_arg_task : public task<Key, F> {
public:
    using key_type = typename task<Key, F>::key_type;
    using arg_type = detail::ntuple_t<key_type, sizeof...(Args)>;
    auto_arg_task(F f, const key_type& key, const arg_type& args)
        : task<Key, F>(f, key)
        , args_(args)
    {}
    auto_arg_task(const key_type& key, const arg_type& args)
        : task<Key, F>(key)
        , args_{ args }
    {}
    auto_arg_task(key_type&& key, arg_type&& args)
        : task<Key, F>(std::move(key))
        , args_{ std::move(args) }
    {}

protected:
    virtual void run()
    {
        run(args_);
    }
    void run(const arg_type& arg)
    {
        detail::call_with_tuple([this](auto && ...args) {this->exec(this->get_pretask_result<Args>(args)...); }, arg);
    }
private:
    arg_type args_;
};

template<class Key, class R, class ...Args>
auto_arg_task(R(*f)(Args...), Key key, detail::ntuple_t<Key, sizeof...(Args)>)->auto_arg_task<std::remove_reference_t<Key>, R(*)(Args...), Args...>;

}
