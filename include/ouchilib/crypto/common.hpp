#pragma once
#include <type_traits>
#include <iterator>
#include <utility>
#include <cassert>
#include <cstdint>

#include "ouchilib/utl/step.hpp"
#include "ouchilib/utl/multiitr.hpp"

namespace ouchi::crypto {

template<class T>
inline void secure_memset(volatile T& ptr, std::uint8_t ch)
{
    using vp = volatile std::uint8_t*;
    auto* dest = reinterpret_cast<vp>(&ptr);
    for (auto i : ouchi::step(sizeof T)) {
        dest[i] = ch;
    }
}
template<class T, size_t N>
inline void secure_memset(volatile T(&array)[N], std::uint8_t ch)
{
    using vp = volatile std::uint8_t*;
    auto* dest = reinterpret_cast<vp>(array);
    for (auto i : ouchi::step(sizeof array)) {
        dest[i] = ch;
    }
}
inline void secure_memset(volatile void* ptr, std::uint8_t ch, size_t size)
{
    auto* dest = static_cast<volatile std::uint8_t*>(ptr);
    for (auto i : ouchi::step(size)) {
        dest[i] = ch;
    }
}

template<class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_unsigned_v<Int>>* = nullptr>
inline constexpr Int rotr(Int x, unsigned nbit)
{
    constexpr unsigned w = sizeof(Int) * CHAR_BIT;
    return (x >> nbit) | (x << (w - nbit));
}
template<class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_unsigned_v<Int>>* = nullptr>
inline constexpr Int rotl(Int x, unsigned nbit)
{
    constexpr unsigned w = sizeof(Int) * CHAR_BIT;
    return (x << nbit) | (x >> (w - nbit));
}

template<size_t>
class memory_view;

template<size_t SizeInByte>
struct memory_entity {
    using value_type = std::uint8_t;

    memory_entity() = default;
    memory_entity(const memory_entity&) = default;
    memory_entity(const void* mem)
    {
        memcpy(data, mem, SizeInByte);
    }
    constexpr memory_entity(const char* mem)
        : data{}
    {
        for (auto i = 0u; i < SizeInByte; ++i)
            data[i] = static_cast<std::uint8_t>(mem[i]);
    }
    // deep copy
    explicit memory_entity(memory_view<SizeInByte> mv);
    memory_entity& operator=(memory_view<SizeInByte> mv) noexcept;
    memory_entity& operator=(const memory_entity&) = default;
    
    std::uint8_t data[SizeInByte];

    /// <summary>
    /// load size byte of data from src to data[to..to+size].
    /// </summary>
    /// <summary>
    /// <returns>loaded size</returns>
    size_t load(const void* src, size_t size, size_t to = 0)
    {
        auto* ptr = reinterpret_cast<const std::uint8_t*>(src);
        size = std::min(size, SizeInByte - to);
        for (auto i = 0u; i < size; ++i) {
            data[to + i] = ptr[i];
        }
        return size;
    }
    constexpr size_t size() const noexcept { return SizeInByte; }
    std::uint8_t& operator[](size_t i) noexcept { return data[i]; }
    const std::uint8_t& operator[](size_t i) const noexcept { return data[i]; }

    friend bool operator==(const memory_entity& lhs, const memory_entity& rhs) noexcept
    {
        for (auto [a, b] : ouchi::multiitr{ lhs.data, rhs.data })
            if (a != b) return false;
        return true;
    }
    friend bool operator!=(const memory_entity& lhs, const memory_entity& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

template<size_t SizeInByte>
class memory_view {
public:
    const std::uint8_t * data;
    using value_type = std::uint8_t;
    memory_view(const void* ptr) : data{ static_cast<const uint8_t*>(ptr) } {}
    memory_view() : data{ nullptr } {}
    memory_view(const memory_entity<SizeInByte>& me) : data{ me.data } {}
    const std::uint8_t& operator[](size_t i) const noexcept {
        assert(data != nullptr);
        return data[i];
    }
    constexpr size_t size() const noexcept { return SizeInByte; }

    friend memory_entity<SizeInByte> operator^(memory_view<SizeInByte> l, memory_view<SizeInByte> r) noexcept
    {
        memory_entity<SizeInByte> me;
        for (auto i : ouchi::step(SizeInByte)) {
            me[i] = l[i] ^ r[i];
        }
        return me;
    }
    template<class Int, std::enable_if_t<std::is_integral_v<Int>>* = nullptr>
    friend memory_entity<SizeInByte> operator^(memory_view<SizeInByte> l, Int r) noexcept
    {
        std::make_unsigned_t<Int> u = r;
        memory_entity<SizeInByte> me;
        for (auto i : ouchi::step(SizeInByte)) {
            me[i] = l[i] ^ (std::uint8_t)u;
            u = u >> 8;
        }
        return me;
    }
    template<class Int, std::enable_if_t<std::is_integral_v<Int>>* = nullptr>
    friend memory_entity<SizeInByte> operator^(Int l, memory_view<SizeInByte> r) noexcept
    {
        return r ^ l;
    }
};

template<size_t SizeInByte>
inline memory_entity<SizeInByte>::memory_entity(memory_view<SizeInByte> mv)
{
    std::memcpy(data, mv.data, SizeInByte);
}

template<size_t SizeInByte>
inline memory_entity<SizeInByte>& memory_entity<SizeInByte>::operator=(memory_view<SizeInByte> mv) noexcept
{
    std::memcpy(data, mv.data, SizeInByte);
    return *this;
}

// global function

namespace detail {

template<size_t ...S>
inline void add_impl(const std::uint8_t* lhs, const std::uint8_t* rhs, std::uint8_t* dest,
                     std::index_sequence<S...>)
{
    ((dest[S] = lhs[S] ^ rhs[S]), ...);
}

template<class Int, size_t ...S>
inline constexpr void unpack_impl(Int src, std::uint8_t* dest, std::index_sequence<S...>)
{
    ((dest[S] = static_cast<std::uint8_t>(src >> (8 * (sizeof(Int) - S - 1)))), ...);
}

template<class Int, std::enable_if_t<std::is_integral_v<Int>>* = nullptr>
inline constexpr void unpack(Int src, void* dest)
{
    auto* ptr = reinterpret_cast<std::uint8_t*>(dest);
    unpack_impl(src, ptr, std::make_index_sequence<sizeof(Int)>{});
}

template<class Int, size_t ...S>
inline constexpr Int pack_impl(const std::uint8_t* src, std::index_sequence<S...>) noexcept
{
    return ((static_cast<Int>(src[S]) << (8 * (sizeof(Int) - S - 1))) | ...);
}

template<class Int, std::enable_if_t<std::is_integral_v<Int>>* = nullptr>
inline constexpr Int pack(const void* src) noexcept
{
    return pack_impl<Int>(reinterpret_cast<const std::uint8_t*>(src),
                          std::make_index_sequence<sizeof(Int)>{});
}

}

template<size_t SizeInByte>
inline void add(memory_view<SizeInByte> operand1, memory_view<SizeInByte> operand2, void* dest) noexcept
{
    auto res = static_cast<std::uint8_t*>(dest);
    detail::add_impl(operand1.data, operand2.data, reinterpret_cast<std::uint8_t*>(dest),
                     std::make_index_sequence<SizeInByte>{});
}

inline void add_assign(void* srcdest, const void* src2, size_t size) noexcept
{
    auto* dest = static_cast<std::uint8_t*>(srcdest);
    auto* src = static_cast<const std::uint8_t*>(src2);

    for (auto i = 0u; i < size; ++i) {
        dest[i] ^= src[i];
    }
}
template<size_t SizeInByte>
inline void add_assign(void* srcdest, memory_view<SizeInByte> src2) noexcept
{
    //add_assign(srcdest, src2.data, SizeInByte);
    detail::add_impl(reinterpret_cast<const std::uint8_t*>(srcdest), src2.data,
                     reinterpret_cast<std::uint8_t*>(srcdest),
                     std::make_index_sequence<SizeInByte>{});
}

template<size_t StepWidth>
struct memory_iterator {
    static_assert(StepWidth);

    std::uint8_t* current;
    size_t size;

    memory_iterator()
        : current{ nullptr }
        , size{ 0 }
    {}
    template<class T>
    memory_iterator(T* ptr, size_t cnt)
        : current{ static_cast<std::uint8_t*>(ptr) }
        , size{ cnt * sizeof(T) }
    {}
    memory_iterator(void* ptr, size_t cnt)
        : current{ static_cast<std::uint8_t*>(ptr) }
        , size{ cnt }
    {}
    template<class T, size_t Size>
    memory_iterator(T(&arr)[Size])
        : current(reinterpret_cast<std::uint8_t*>(arr))
        , size{ Size * sizeof(T) }
    {}

    memory_iterator& operator++() noexcept
    {
        assert(size >= StepWidth);
        current += StepWidth;
        size -= StepWidth;
        return *this;
    }
    memory_iterator operator++(int) noexcept
    {
        auto cp = *this;
        ++(*this);
        return cp;
    }
    memory_iterator& operator--() noexcept
    {
        current -= StepWidth;
        size += StepWidth;
        return *this;
    }
    memory_iterator operator--(int) noexcept
    {
        auto cp = *this;
        --(*this);
        return cp;
    }
    memory_view<StepWidth> operator*() const noexcept
    {
        return memory_view<StepWidth>{current};
    }
    friend memory_iterator operator+(memory_iterator i, long long d)
    {
        assert((d > 0 && i.count() >= (unsigned long long)d) || d <= 0);
        return memory_iterator<StepWidth>(i.current + (d * StepWidth),
                                          i.size - d * StepWidth);
    }
    friend memory_iterator operator+(long long d, memory_iterator i)
    {
        return i + d;
    }
    friend memory_iterator operator-(memory_iterator i, long long d)
    {
        return memory_iterator<StepWidth>(i.current - (d * StepWidth),
                                          i.size + d * StepWidth);
    }
    friend memory_iterator operator-(long long d, memory_iterator i)
    {
        return i - d;
    }
    friend size_t operator-(memory_iterator lhs, memory_iterator rhs)
    {
        return lhs.current - rhs.current;
    }
    friend bool operator==(memory_iterator lhs, memory_iterator rhs)
    {
        return lhs.current == rhs.current;
    }
    friend bool operator!=(memory_iterator lhs, memory_iterator rhs)
    {
        return !(lhs == rhs);
    }
    void* raw() noexcept { return current; }
    size_t count() const noexcept
    {
        return size / StepWidth;
    }
};

}
