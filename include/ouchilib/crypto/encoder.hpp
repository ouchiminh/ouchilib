#pragma once
#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include "cipher_mode.hpp"

namespace ouchi::crypto {

template<
    template<class> class CipherMode,
    class Algorithm,
    std::enable_if_t<is_crypto_algorithm<Algorithm>::value>* = nullptr>
class encoder {
public:
    template<class ...Args>
    encoder(Args&& ...args)
        : cipher_device(std::forward<Args>(args)...)
    {}
    ///<returns>crypto size</returns>
    size_t encrypt(const void* src, size_t size, void* dest, size_t dest_size)
    {
        auto padsize = Algorithm::block_size - size % Algorithm::block_size;
        if (padsize > dest_size - size) {
            throw std::out_of_range("dest is too short");
        }
        std::memmove(dest, src, size);
        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        // padding
        {
            for (auto i : ouchi::step(padsize)) {
                destptr[i + size] = (std::uint8_t)padsize;
            }
            dest_size = size + padsize;
        }
        for (auto i : ouchi::step(0ull, dest_size, Algorithm::block_size)) {
            cipher_device.encrypt(destptr + i, destptr + i);
        }
        return dest_size;
    }
    ///<returns>plain txt size</returns>
    size_t decrypt(const void* src, size_t size, void* dest, size_t dest_size)
    {
        if (size % Algorithm::block_size)
            throw std::invalid_argument("invalid length : size must be multiple of block size");
        if (dest_size < size)
            throw std::out_of_range("dest is too short");

        dest_size = size;

        auto destptr = reinterpret_cast<std::uint8_t*>(dest);
        auto srcptr = reinterpret_cast<const std::uint8_t*>(src);
        for (auto i : ouchi::step(0ull, dest_size, Algorithm::block_size)) {
            cipher_device.decrypt(srcptr + i, destptr + i);
        }
        // delete pad
        auto padsize = destptr[dest_size - 1];
        return dest_size - padsize;
    }

    template<class T>
    void encrypt(std::basic_istream<T> in, std::basic_ostream<T> out)
    {
        static_assert(Algorithm::block_size % sizeof(T) == 0);
    }
    template<class T>
    void decrypt(std::basic_istream<T> in, std::basic_ostream<T> out)
    {
        static_assert(Algorithm::block_size % sizeof(T) == 0);
    }
private:
    CipherMode<Algorithm> cipher_device;
};

}
