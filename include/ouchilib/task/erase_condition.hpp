#pragma once
#include <functional>
#include <variant>
#include <utility>
#include <chrono>

namespace ouchi::task {

class erase_condition {
    friend class task;
    enum condition_type { time, usr_func, count, constant };

    condition_type type_;
    std::variant<std::chrono::high_resolution_clock::time_point,
                 std::function<bool()>,
                 std::pair<size_t, size_t>, // 累計呼び出し回数, 削除回数
                 bool> condition_;
    bool latest_ = false;

protected:
    // 死ぬならtrue
    [[nodiscard]]
    bool update_cond(const std::chrono::high_resolution_clock::time_point& cond)
    {
        return latest_ = (std::chrono::high_resolution_clock::now() >= cond);
    }
    [[nodiscard]]
    bool update_cond(std::function<bool()>& cond)
    {
        return (latest_ = cond());
    }
    [[nodiscard]]
    bool update_cond(std::pair<std::size_t, size_t>& cond) noexcept
    {
        return latest_ = (++cond.first > cond.second);
    }
    [[nodiscard]]
    constexpr bool update_cond(bool val) noexcept
    {
        return latest_ = val;
    }
public:
    // 常にfalse
    constexpr erase_condition()
        : type_{condition_type::constant}
        , condition_{ false }
    {}
    constexpr erase_condition(std::bool_constant<true>)
        : type_{ condition_type::constant }
        , condition_{ true }
    {}

    // 経過時間で消える条件を設定するコンストラクタ
    template<class Rep, class Period>
    constexpr explicit erase_condition(const std::chrono::duration<Rep, Period>& survival_time)
        : type_{ condition_type::time }
        , condition_{std::chrono::high_resolution_clock::now() + survival_time}
    {}
    // 呼び出し回数で消える条件を設定するコンストラクタ
    constexpr explicit erase_condition(size_t count)
        : type_{ condition_type::count }
        , condition_{ std::make_pair(static_cast<size_t>(0), count) }
    {}

    // 条件を満たすか判定する関数を受け取るコンストラクタ
    constexpr erase_condition(std::function<bool()>&& condition)
        : type_{ condition_type::usr_func }
        , condition_{ std::move(condition) }
    {}
    constexpr erase_condition(const std::function<bool()>& condition)
        : type_{ condition_type::usr_func }
        , condition_{ condition }
    {}

    // 死ぬならtrue
    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!*this;
    }
    // 生き残るならtrue
    [[nodiscard]]
    constexpr bool operator !() const noexcept
    {
        return !latest_;
    }

private:
    // 死ぬならtrue
    constexpr bool update_cond()
    {
        switch (type_) {
        case condition_type::time:
            return update_cond(std::get<condition_type::time>(condition_));
        case condition_type::usr_func:
            return update_cond(std::get<condition_type::usr_func>(condition_));
        case condition_type::count:
            return update_cond(std::get<condition_type::count>(condition_));
        case condition_type::constant:
            return update_cond(std::get<condition_type::constant>(condition_));
        }
        return false;
    }
};

template<class Clock>
erase_condition time_based_erase_cond(const typename Clock::duration& survival_time)
{
    auto deadline = Clock::now() + survival_time;
    auto f = [deadline]() {return Clock::now() >= deadline; };
    return erase_condition{ f };
}

template<class Clock>
erase_condition time_based_erase_cond(const typename Clock::time_point& t)
{
    auto f = [t]() {return Clock::now() >= t; };
    return erase_condition{ f };
}

}
