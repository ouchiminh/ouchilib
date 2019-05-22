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
auto dereference(const std::tuple<Args...>& t)
{
    std::apply(dereference_impl, t);
}

template<class ...Args>
auto dereference(std::tuple<Args...>& t)
{
    return std::apply(dereference_impl, t);
}

const auto increament_impl = [](auto& ...args) {
    return std::tie((++args)...);
};

template<class ...Args>
auto increament(std::tuple<Args...>& t)
{
    return std::apply(increament_impl, t);
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
auto get_begin(std::tuple<Args...>& t)
{
    return std::apply(get_begin_impl, t);
}
template<class ...Args>
auto get_begin(const std::tuple<Args...>& t)
{
    return std::apply(get_begin_impl, t);
}
template<class ...Args>
auto get_end(std::tuple<Args...>& t)
{
    return std::apply(get_end_impl, t);
}
template<class ...Args>
auto get_end(const std::tuple<Args...>& t)
{
    return std::apply(get_end_impl, t);
}

template<class ...Args>
bool equal(Args&& ...args){
    return (args & ...) == (args | ...);
}

template<class C>
size_t get_size(const C& c) { return c.size(); }
template<class T, size_t Size>
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
        iterator(std::tuple<Containers& ...>& containers)
            : itrs_(detail::get_begin(containers))
        {}
        iterator(itr_t&& iterators)
            : itrs_(std::move(iterators))
        {}

        friend bool operator==(const iterator& a, const iterator& b)
        {
            return a.itrs_ == b.itrs_;
        }
        friend bool operator!=(const iterator& a, const iterator& b)
        {
            return !(a == b);
        }
        auto operator*()
        {
            return detail::dereference(itrs_);
        }
        iterator& operator++() {
            itrs_ = detail::increament(itrs_);
            return *this;
        }
        iterator operator++(int)
        {
            auto cp = *this;
            ++(*this);
            return cp;
        }
    };

    auto begin() { return iterator{c_}; }
    auto end() { return iterator{detail::get_end(c_)}; }
};

}
