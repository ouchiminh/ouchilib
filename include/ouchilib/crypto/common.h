#pragma once
#include <type_traits>
#include <iterator>
#include <utility>
#include <cassert>
#include <cstdint>

#include "ouchilib/utl/step.hpp"
#include "ouchilib/utl/multiitr.hpp"

namespace ouchi::crypto {
template<size_t>
class memory_view;

template<size_t SizeInByte>
struct memory_entity {
    using value_type = std::uint8_t;

    memory_entity() = default;
    memory_entity(const memory_entity&) = default;
    memory_entity(void* mem)
    {
        std::memcpy(data, mem, SizeInByte);
    }
    // deep copy
    explicit memory_entity(memory_view<SizeInByte> mv);
    memory_entity& operator=(memory_view<SizeInByte> mv) noexcept;
    memory_entity& operator=(const memory_entity&) = default;
    
    std::uint8_t data[SizeInByte];
    std::uint8_t& operator[](size_t i) noexcept { return data[i]; }
    const std::uint8_t& operator[](size_t i) const noexcept { return data[i]; }
};
template<size_t SizeInByte>
class memory_view {
public:
    const std::uint8_t * data;
    using value_type = std::uint8_t;
    memory_view(const void* ptr) : data{ reinterpret_cast<const uint8_t*>(ptr) } {}
    memory_view() : data{ nullptr } {}
    memory_view(const memory_entity<SizeInByte>& me) : data{ me.data } {} // meの寿命はプログラマーの責任
    const std::uint8_t& operator[](size_t i) const noexcept {
        assert(data != nullptr);
        return data[i];
    }

    friend memory_entity<SizeInByte> operator^(memory_view l, memory_view r) noexcept
    {
        memory_entity<SizeInByte> me;
        for (auto i : ouchi::step(SizeInByte)) {
            me[i] = l[i] ^ r[i];
        }
        return me;
    }
    template<class Int>
    friend memory_entity<SizeInByte> operator^(memory_view l, Int r) noexcept
    {
        std::make_unsigned_t<Int> u = r;
        memory_entity<SizeInByte> me;
        for (auto i : ouchi::step(SizeInByte)) {
            me[i] = l[i] ^ r;
            r = r >> 8;
        }
        return me;
    }
    template<class Int>
    friend memory_entity<SizeInByte> operator^(Int l, memory_view r) noexcept
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

template<size_t SizeInByte>
void add(memory_view<SizeInByte> operand1, memory_view<SizeInByte> operand2, void* dest) noexcept
{
    auto res = reinterpret_cast<std::uint8_t*>(dest);
    for (auto i : ouchi::step(SizeInByte)) {
        res[i] = operand1[i] ^ operand2[i];
    }
}

void add_assign(void* srcdest, void* src2, size_t size) noexcept
{
    auto* dest = reinterpret_cast<std::uint8_t*>(srcdest);
    auto* src = reinterpret_cast<std::uint8_t*>(src2);

    for (auto&& i : ouchi::step(size)) {
        dest[i] ^= src[i];
    }
}

template<size_t StepWidth>
struct memory_iterator {
    void* current;
    size_t size;

    memory_iterator()
        : current{ nullptr }
        , size{ 0 }
    {}
    template<class T>
    memory_iterator(T* ptr, size_t cnt)
        : current{ ptr }
        , size{ cnt * sizeof(T) }
    {}
    template<class T, size_t Size>
    memory_iterator(T(&arr)[Size])
        : current(arr)
        , size{ Size * sizeof(T) }
    {}

    memory_iterator& operator++() noexcept
    {
        assert(size > StepWidth);
        current += StepWidth;
        size -= StepWidth;
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
    void* raw() noexcept { return current; }
};

}
