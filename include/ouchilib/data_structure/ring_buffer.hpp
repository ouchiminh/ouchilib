#pragma once
#include <memory>
#include <initializer_list>
#include <cstdint>

namespace ouchi {

template<class T, class Allocator = std::allocator<T>>
class ring_buffer {
    template<class T>
    class basic_iterator{};
public:
    using size_type = std::uintptr_t;
    using difference_type = std::intptr_t;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    class iterator;
    class const_iterator;

    ring_buffer();
    ring_buffer(size_type capacity);
    ring_buffer(size_type n, const_reference value);
    ring_buffer(std::initializer_list<T> il);
    ring_buffer(const_iterator first, const_iterator last);
private:
    size_type size_;
    size_type offset_;
    T* data_;
};

}
