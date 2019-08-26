#pragma once
#include <type_traits>
#include <utility>

#include "common.hpp"

namespace ouchi::crypto {
template<class T, class = void>
struct is_crypto_algorithm : std::false_type {};

template<class T>
struct is_crypto_algorithm<T,
    std::void_t<
    decltype(T::encrypt),
    decltype(T::decrypt),
    decltype(T::block_size),
    typename T::block_t
    >
>
    : std::true_type {};


template<class A>
struct ecb {
    using block_t = typename A::block_t;
    static constexpr size_t block_size = A::block_size;
    static constexpr bool is_encrypt_parallelizable = true;
    static constexpr bool is_decrypt_parallelizable = true;

    A encoder;

    template<class ...Args>
    ecb(std::in_place_t, Args&& ...args)
        : encoder{std::forward<Args>(args)...}
    {}

    void encrypt(block_t src, void* dest)
    {
        encoder.encrypt(src, dest);
    }
    void decrypt(block_t src, void* dest)
    {
        encoder.decrypt(src, dest);
    }
    void set_encrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before) {}
    void set_decrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before) {}
};

template<class A>
struct cbc {
    using block_t = typename A::block_t;
    static constexpr size_t block_size = A::block_size;
    static constexpr bool is_encrypt_parallelizable = false;
    static constexpr bool is_decrypt_parallelizable = true;

    A encoder;
    memory_entity<block_size> vector;
    template<class ...Args>
    cbc(std::in_place_t, memory_view<block_size> iv, Args&& ...args)
        : encoder{std::forward<Args>(args)...}
        , vector(iv)
    {}
    void encrypt(block_t src, void* dest)
    {
        encoder.encrypt(block_t(vector) ^ src, vector.data);
        std::memcpy(dest, vector.data, block_size);
    }
    void decrypt(block_t src, void* dest)
    {
        encoder.decrypt(src, dest);
        add_assign<block_size>(dest, vector);
        vector = src;
    }
    void set_decrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before)
    {
        vector = *just_before;
    }
};

template<class A>
struct ctr {
    using block_t = typename A::block_t;
    static constexpr size_t block_size = A::block_size;
    static constexpr bool is_encrypt_parallelizable = true;
    static constexpr bool is_decrypt_parallelizable = true;

    A encoder;
    memory_entity<block_size> nonce;
    size_t counter;

    template<class ...Args>
    ctr(std::in_place_t, memory_view<block_size> init_nonce, size_t init_ctr, Args&& ...args)
        : encoder{ std::forward<Args>(args)... }
        , nonce{ init_nonce }
        , counter{ init_ctr }
    {}
    void encrypt(block_t src, void* dest)
    {
        memory_entity<block_size> buf;
        encoder.encrypt(memory_view<block_size>(nonce) ^ counter, buf.data);
        add<block_size>(buf, src, dest);
        update_counter();
    }
    void decrypt(block_t src, void* dest)
    {
        encrypt(src, dest);
    }
    virtual void update_counter()
    {
        ++counter;
    }
    virtual void update_counter(size_t current_cnt)
    {
        counter = current_cnt;
    }
    void set_decrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before)
    {
        update_counter(first.count() - just_before.count());
        update_counter();
    }
    void set_encrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before)
    {
        set_decrypt_state(first, just_before);
    }
};
} // namespace ouchi::crypto
