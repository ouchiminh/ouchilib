#pragma once
#include <type_traits>
#include <optional>
#include <string_view>
#include <variant>

namespace ouchi::result {

template<class T>
struct ok{
    template<class U, std::enable_if_t<std::is_same_v<std::remove_reference_t<U>,
                                                      std::remove_reference_t<T>>>* = nullptr>
    constexpr ok(U& v) : value(v) {};
    template<class U, std::enable_if_t<std::is_same_v<std::remove_reference_t<U>,
                                                      std::remove_reference_t<T>>>* = nullptr>
    constexpr ok(U&& v) : value(v) {};
    T value;
};

template<class U>
ok(U&)->ok<U&>;
template<class U>
ok(U&&)->ok<std::remove_reference_t<U>>;

template<class T>
struct err{
    template<class U, std::enable_if_t<std::is_same_v<std::remove_reference_t<U>,
                                                      std::remove_reference_t<T>>>* = nullptr>
    constexpr err(U& v) : value(v) {};
    template<class U, std::enable_if_t<std::is_same_v<std::remove_reference_t<U>,
                                                      std::remove_reference_t<T>>>* = nullptr>
    constexpr err(U&& v) : value(v) {};
    T value;
};

template<class U>
err(U&)->err<U&>;
template<class U>
err(U&&)->err<std::remove_reference_t<U>>;

/// <summary>
/// 処理の結果を報告するためのクラス。
/// </summary>
/// <typeparam name="T">エラーではない結果の型</typeparam>
/// <typeparam name="Err">エラーの結果の型</typeparam>
/// <remarks>
/// `T`と`Err`は共にCopyConstructibleである必要がある。
/// </remarks>
template<class T = std::monostate, class Err = std::monostate, class = void>
class [[nodiscard]] result;

template<class T, class Err>
class [[nodiscard]] result<T, Err,
                           std::enable_if_t<std::is_destructible_v<T> &&
                                            std::is_destructible_v<Err>>>
{
public:
    using ok_type = std::conditional_t<std::is_reference_v<T>, std::reference_wrapper<std::remove_reference_t<T>>, T>;
    using err_type = std::conditional_t<std::is_reference_v<Err>, std::reference_wrapper<std::remove_reference_t<Err>>, Err>;

    template<class U = T,
             std::enable_if_t<std::is_convertible_v<std::remove_reference_t<U>, std::remove_reference_t<T>>>* = nullptr>
    constexpr result(::ouchi::result::ok<U>&& success)
        : data_{ std::in_place_index<0>, success.value }
    {}
    template<class U = T,
             std::enable_if_t<std::is_convertible_v<std::remove_reference_t<U>, std::remove_reference_t<Err>>>* = nullptr>
    constexpr result(::ouchi::result::err<U>&& error)
        : data_{ std::in_place_index<1>, error.value }
    {}
    template<class U = T,
             std::enable_if_t<std::is_convertible_v<std::remove_reference_t<U>, std::remove_reference_t<T>>>* = nullptr>
    constexpr result(::ouchi::result::ok<U&>&& success)
        : data_{ std::in_place_index<0>, success.value }
    {}
    template<class U = Err,
             std::enable_if_t<std::is_convertible_v<std::remove_reference_t<U>, std::remove_reference_t<Err>>>* = nullptr>
    constexpr result(::ouchi::result::err<U&>&& error)
        : data_{ std::in_place_index<1>, error.value }
    {}
    result(const result&) = default;
    result(result&&) = default;
    result& operator=(const result&) = default;
    result& operator=(result&&) = default;

    constexpr bool is_ok() const noexcept
    {
        return data_.index() == 0;
    }
    constexpr bool is_err() const noexcept
    {
        return !is_ok();
    }
    constexpr operator bool() const noexcept
    {
        return is_ok();
    }
    template<class T2>
    friend constexpr result<T2, Err> operator&&(const result<T, Err>& lhs, const result<T2, Err> rhs) noexcept
    {
        return lhs.is_ok() ? rhs : ::ouchi::result::err(lhs.unwrap_err());
    }
    friend constexpr result<T, Err> operator||(const result<T, Err>& lhs, const result<T, Err> rhs) noexcept
    {
        return lhs.is_ok() ? lhs : rhs;
    }

    constexpr std::optional<ok_type> ok() const noexcept
    {
        if (is_ok()) return std::get<0>(data_);
        return std::nullopt;
    }
    constexpr std::optional<err_type> err() const noexcept
    {
        if (is_err()) return std::get<1>(data_);
        return std::nullopt;
    }

    const T& unwrap() const
    {
        return std::get<0>(data_);
    }
    const Err& unwrap_err() const
    {
        return std::get<1>(data_);
    }

    constexpr const T& unwrap_or(const ok_type& ok_val) const noexcept
    {
        if (is_err()) return ok_val;
        return unwrap();
    }
    template<class F, std::enable_if_t<std::is_invocable_r_v<ok_type, F, err_type>>* = nullptr>
    auto unwrap_or_else(F&& closure) const noexcept(noexcept(std::declval<F&>()(std::declval<err_type&>())))
        ->std::common_type_t<decltype(std::declval<F&>()(std::declval<err_type&>())), const ok_type&>
    {
        if (is_err()) return closure(err().value());
        return unwrap();
    }

    template<class Exc = std::logic_error>
    const T& expect(const std::string& exception_msg) const
    {
        if (is_ok()) return unwrap();
        throw Exc(exception_msg);
    }
    template<class Exc = std::logic_error>
    const Err& expect_err(std::string exception_msg) const
    {
        if (is_err()) return unwrap();
        throw Exc(exception_msg);
    }

    template<class Op>
    auto map(Op&& op) const noexcept(noexcept(std::declval<Op&>()(std::declval<ok_type&>())))
        -> std::enable_if_t<
            std::is_invocable_v<Op, ok_type>,
            result<std::invoke_result_t<Op, ok_type>, Err>
        >
    {
        if (is_err()) return ::ouchi::result::err((Err)unwrap_err());
        return ::ouchi::result::ok(op(unwrap()));
    }
    template<class Op>
    auto map_err(Op&& op) const noexcept(noexcept(std::declval<Op&>()(std::declval<err_type&>())))
        -> std::enable_if_t<
            std::is_invocable_v<Op, err_type>,
            result<ok_type, std::invoke_result_t<Op, err_type>>
        >
    {
        if (is_ok()) return ::ouchi::result::ok(unwrap());
        return ::ouchi::result::err(op(unwrap_err()));
    }

private:
    std::variant<ok_type, err_type> data_;
};

}
