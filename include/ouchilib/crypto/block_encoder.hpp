#pragma once
#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <algorithm>
#include "cipher_mode.hpp"
#include "ouchilib/thread/thread-pool.hpp"

namespace ouchi::crypto {

template<
    template<class> class CipherMode,
    class Algorithm,
    std::enable_if_t<is_crypto_algorithm<Algorithm>::value>* = nullptr>
class block_encoder {
public:
    template<class ...Args>
    block_encoder(Args&& ...args)
        : cipher_device_(std::forward<Args>(args)...)
    {}
    ///<returns>crypto size</returns>
    size_t encrypt(const void* src, size_t size, void* dest, size_t dest_size)
    {
        auto padsize = check_edestsize(size, dest_size);

        std::memmove(dest, src, size);
        dest_size = size + padsize;

        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        pad(destptr, size, padsize);
        for (auto i : ouchi::step(0ull, dest_size, Algorithm::block_size)) {
            cipher_device_.encrypt(destptr + i, destptr + i);
        }
        return dest_size;
    }
    ///<returns>plain txt size</returns>
    size_t decrypt(const void* src, size_t size, void* dest, size_t dest_size)
    {
        check_ddestsize(size, dest_size);
        dest_size = size;

        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        auto srcptr = reinterpret_cast<const std::uint8_t*>(src);
        for (auto i : ouchi::step(0ull, dest_size, Algorithm::block_size)) {
            cipher_device_.decrypt(srcptr + i, destptr + i);
        }
        // delete pad
        auto padsize = destptr[dest_size - 1];
        check_pad(padsize, destptr + dest_size - padsize);
        return dest_size - padsize;
    }
    template<class Cm = CipherMode<Algorithm>, std::enable_if_t<Cm::is_encrypt_parallelizable>* = nullptr>
    size_t encrypt_parallel(const void* src, size_t size,
                            void* dest, size_t dest_size,
                            unsigned thread_cnt)
    {
        assert(thread_cnt);
        auto device = cipher_device_;
        auto padsize = check_edestsize(size, dest_size);
        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        ouchi::thread::thread_pool tp(thread_cnt);

        std::memmove(dest, src, size);
        dest_size = size + padsize;
        pad(dest, size, padsize);
        {
            const memory_iterator<Algorithm::block_size> first(dest, dest_size);
            const memory_iterator<Algorithm::block_size> last(destptr + dest_size, 0);
            memory_iterator<Algorithm::block_size> it(dest, dest_size);
            auto width = first.count() / thread_cnt;
            for (auto i : ouchi::step(thread_cnt - 1)) {
                tp.push([device, first, it, width]() mutable {
                    auto end = it + width;
                    if(it != first) device.set_encrypt_state(first, it - 1);
                    for (; it != end; ++it) {
                        device.encrypt(*it, it.raw());
                    }
                });
                it = it + width;
            }
            tp.push([device, first, it, last, dest]() mutable {
                if (it != first) device.set_encrypt_state(first, it - 1);
                device.set_encrypt_state(first, it - 1);
                for (; it != last; ++it) {
                    device.encrypt(*it, it.raw());
                }
            });
        }
        tp.wait();
        return dest_size;
    }
    template<class Cm = CipherMode<Algorithm>, std::enable_if_t<Cm::is_decrypt_parallelizable>* = nullptr>
    size_t decrypt_parallel(const void* src, size_t size,
                            void* dest, size_t dest_size,
                            unsigned thread_cnt)
    {
        assert(thread_cnt);
        check_ddestsize(size, dest_size);
        dest_size = size;
        ouchi::thread::thread_pool tp(thread_cnt);
        auto device = cipher_device_;
        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        auto srcptr = reinterpret_cast<const std::uint8_t*>(src);

        // decryption
        {
            const memory_iterator<Algorithm::block_size> first(dest, dest_size);
            const memory_iterator<Algorithm::block_size> last(destptr + dest_size, 0);
            memory_iterator<Algorithm::block_size> it(dest, dest_size);
            auto width = first.count() / thread_cnt;
            for (auto i : ouchi::step(thread_cnt - 1)) {
                tp.push([device, first, it, width]() mutable {
                    auto end = it + width;
                    if(it != first) device.set_decrypt_state(first, it - 1);
                    for (; it != end; ++it) {
                        device.decrypt(*it, it.raw());
                    }
                });
                it = it + width;
            }
            tp.push([device, first, it, last, dest]() mutable {
                if (it != first) device.set_decrypt_state(first, it - 1);
                device.set_decrypt_state(first, it - 1);
                for (; it != last; ++it) {
                    device.decrypt(*it, it.raw());
                }
            });
        }

        // delete pad
        auto padsize = destptr[dest_size - 1];
        check_pad(padsize, destptr + dest_size - padsize);
        return dest_size - padsize;
    }
    void encrypt(std::istream& in, std::ostream& out)
    {
        constexpr auto cnt = Algorithm::block_size;
        memory_entity<cnt> block;
        while (in.read(reinterpret_cast<char*>(block.data), cnt).gcount() == cnt) {
            cipher_device_.encrypt(block.data, block.data);
            out.write(reinterpret_cast<char*>(block.data), cnt);
        }
        auto padbegin = in.gcount();
        auto padsize = cnt - padbegin;
        std::fill(block.data + padbegin, std::end(block.data), (std::uint8_t)padsize);
        cipher_device_.encrypt(block.data, block.data);
        out.write(reinterpret_cast<char*>(block.data), cnt);
    }
    void decrypt(std::istream& in, std::ostream& out)
    {
        constexpr auto cnt = Algorithm::block_size;
        memory_entity<Algorithm::block_size> block;
        while (true) {
            in.read(reinterpret_cast<char*>(block.data), cnt);
            if (in.gcount() != cnt) throw std::runtime_error("decryption failed. invalid length");
            cipher_device_.decrypt(block.data, block.data);
            in.ignore(1);
            if (!in.eof()) {
                out.write(reinterpret_cast<char*>(block.data), cnt);
            } else {
                auto padsize = block[cnt - 1];
                check_pad(padsize, block.data + cnt - padsize);
                out.write(reinterpret_cast<char*>(block.data), cnt - padsize);
                break;
            }
            in.seekg(-1, std::ios_base::cur);
        }
    }
private:
    static void pad(void* dest, size_t srcsize, size_t padsize)
    {
        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        for (auto i : ouchi::step(padsize)) {
            destptr[i + srcsize] = (std::uint8_t)padsize;
        }
    }
    static size_t check_edestsize(size_t srcsize, size_t dest_size)
    {
        auto padsize = Algorithm::block_size - srcsize % Algorithm::block_size;
        if(padsize > dest_size - srcsize)
            throw std::out_of_range("dest is too short");
        return padsize;
    }
    static void check_ddestsize(size_t srcsize, size_t dest_size)
    {
        if (srcsize % Algorithm::block_size)
            throw std::invalid_argument("invalid length : size must be multiple of block size");
        if (dest_size < srcsize)
            throw std::out_of_range("dest is too short");
    }
    static void check_pad(size_t padsize, std::uint8_t* padbegin)
    {
        if(!(padsize <= Algorithm::block_size &&
             std::count(padbegin, padbegin + padsize, (std::uint8_t)padsize) == padsize))
            throw std::runtime_error("decryption failed. please check key or initial condition.");
    }
    CipherMode<Algorithm> cipher_device_;
};

}
