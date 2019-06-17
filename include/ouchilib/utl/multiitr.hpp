#pragma once
#include <type_traits>
#include <tuple>
#include <iterator>
#include <stdexcept>
#include "meta-helper.hpp"

namespace ouchi {

namespace detail {

const auto dereference_impl = [](auto&& ...args)
{
    return std::tie((*args)...);
};
template<class ...Args>
[[nodiscard]]
decltype(auto) dereference(const std::tuple<Args...>& t)
{
    std::apply(dereference_impl, t);
}

template<class ...Args>
[[nodiscard]]
decltype(auto) dereference(std::tuple<Args...>& t)
{
    return std::apply(dereference_impl, t);
}

const auto increament_impl = [](auto& ...args) {
    ((++args), ...);
};

template<class ...Args>
auto increament(std::tuple<Args...>& t)
{
    return std::apply(increament_impl, t);
}

const auto decreament_impl = [](auto& ...args) {
    ((--args), ...);
};

template<class ...Args>
auto decreament(std::tuple<Args...>& t)
{
    return std::apply(decreament_impl, t);
}

const auto get_begin_impl = [](auto&& ...args) {
    using std::begin;
    return std::make_tuple(begin(args)...);
};
const auto get_end_impl = [](auto&& ...args) {
    using std::end;
    return std::make_tuple(end(args)...);
};

template<class ...Args>
[[nodiscard]]
auto get_begin(std::tuple<Args...>& t)
{
    return std::apply(get_begin_impl, t);
}
template<class ...Args>
[[nodiscard]]
auto get_begin(const std::tuple<Args...>& t)
{
    return std::apply(get_begin_impl, t);
}
template<class ...Args>
[[nodiscard]]
auto get_end(std::tuple<Args...>& t)
{
    return std::apply(get_end_impl, t);
}
template<class ...Args>
[[nodiscard]]
auto get_end(const std::tuple<Args...>& t)
{
    return std::apply(get_end_impl, t);
}

template<class ...Args>
[[nodiscard]]
bool equal(Args&& ...args){
    return (args & ...) == (args | ...);
}

template<class C>
[[nodiscard]]
size_t get_size(const C& c) { return c.size(); }
template<class T, size_t Size>
[[nodiscard]]
constexpr size_t get_size(const T(&)[Size]) { return Size; }

}// namespace detail

template<class ...Containers>
class multiitr {
    static_assert(std::conjunction_v<has_iterator<Containers>...>);
    static_assert(std::conjunction_v<has_size<Containers>...>);

    std::tuple<Containers&...> c_;
public:
    multiitr(Containers& ...args)
        : c_(std::tie(args...))
    {
        if (!detail::equal(detail::get_size(args)...))
            throw std::runtime_error("all container must have same size");

    }

    class iterator {
        using itr_t = std::tuple<his_iterator_t<Containers>...>;
        itr_t itrs_;

    public:
        using difference_type = std::common_type_t<typename std::iterator_traits<his_iterator_t<Containers>>::difference_type...>;

        iterator(const std::tuple<Containers& ...>& containers)
            : itrs_(detail::get_begin(containers))
        {}
        iterator(itr_t&& iterators)
            : itrs_(std::move(iterators))
        {}

        [[nodiscard]]
        friend bool operator==(const iterator& a, const iterator& b)
        {
            return a.itrs_ == b.itrs_;
        }
        [[nodiscard]]
        friend bool operator!=(const iterator& a, const iterator& b)
        {
            return !(a == b);
        }
        [[nodiscard]]
        decltype(auto) operator*()
        {
            return detail::dereference(itrs_);
        }
        iterator& operator++() {
            detail::increament(itrs_);
            return *this;
        }
        iterator operator++(int)
        {
            auto cp = *this;
            ++(*this);
            return cp;
        }
        template<class It = iterator,
                 std::enable_if_t<std::conjunction_v<
                     std::is_base_of<std::bidirectional_iterator_tag,
                                     typename std::iterator_traits<his_iterator_t<Containers>>::iterator_category>...
                 >>* = nullptr>
        iterator& operator--()
        {
            detail::decreament(itrs_);
            return *this;
        }
        template<class It = iterator,
                 std::enable_if_t<std::conjunction_v<
                     std::is_base_of<std::bidirectional_iterator_tag,
                                     typename std::iterator_traits<his_iterator_t<Containers>>::iterator_category>...
                 >>* = nullptr>
            iterator& operator--(int)
        {
            auto cp = *this;
            --(*this);
            return cp;
        }
    };
    [[nodiscard]]
    auto begin() { return iterator{c_}; }
    [[nodiscard]]
    auto end() { return iterator{detail::get_end(c_)}; }
};

}
