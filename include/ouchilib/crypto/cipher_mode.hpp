#pragma once
#include <type_traits>
#include <utility>

#include "common.h"

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


template<class A, std::enable_if_t<is_crypto_algorithm<A>::value>* = nullptr>
struct ecb {
    using block_t = typename A::block_t;
    static constexpr size_t block_size = A::block_size;
    static constexpr bool is_encrypt_parallelizable = true;
    static constexpr bool is_decrypt_parallelizable = true;

    A encoder;

    template<class ...Args>
    ecb(Args&& ...args)
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

template<class A, std::enable_if_t<is_crypto_algorithm<A>::value>* = nullptr>
struct cbc {
    using block_t = typename A::block_t;
    static constexpr size_t block_size = A::block_size;
    static constexpr bool is_encrypt_parallelizable = false;
    static constexpr bool is_decrypt_parallelizable = true;

    A encoder;
    memory_entity<block_size> vector;
    template<class ...Args>
    cbc(memory_view<block_size> iv, Args&& ...args)
        : encoder{std::forward<Args>(args)...}
        , vector(iv)
    {}
    void encrypt(block_t src, void* dest)
    {
        encoder.encrypt(vector ^ src, vector.data);
        std::memcpy(dest, vector.data, block_size);
    }
    void decrypt(block_t src, void* dest)
    {
        encoder.decrypt(src, dest);
        add_assign(dest, vector)
        vector = src;
    }
    void set_decrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before)
    {
        vector = *just_before;
    }
};

template<class A, std::enable_if_t<is_crypto_algorithm<A>::value>* = nullptr>
struct ctr {
    using block_t = typename A::block_t;
    static constexpr size_t block_size = A::block_size;
    static constexpr bool is_encrypt_parallelizable = true;
    static constexpr bool is_decrypt_parallelizable = true;

    A encoder;
    memory_entity<block_size> nonce;
    size_t counter;

    template<class ...Args>
    ctr(memory_view<block_size> init_nonce, size_t init_ctr, Args&& ...args)
        : encoder{ std::forward<Args>(args)... }
        , nonce{ init_nonce }
        , ctr{ init_ctr }
    {}
    void encrypt(block_t src, void* dest)
    {
        memory_entity<block_size> buf;
        encoder.encrypt(nonce ^ counter, buf.data);
        add(buf, src, dest);
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
        update_counter(first.size - just_before.size);
    }
    void set_encrypt_state(memory_iterator<block_size> first, memory_iterator<block_size> just_before)
    {
        set_decrypt_state(first, just_before);
    }
};
} // namespace ouchi::crypto
