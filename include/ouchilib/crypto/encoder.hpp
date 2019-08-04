#pragma once
#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <algorithm>
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
        if (check_pad(padsize, destptr + dest_size - padsize))
            throw std::runtime_error("decryption failed. please check key or initial condition.");
        return dest_size - padsize;
    }

    void encrypt(std::istream& in, std::ostream& out)
    {
        constexpr auto cnt = Algorithm::block_size;
        memory_entity<cnt> block;
        while (in.read(block.data, cnt).gcount() == cnt) {
            cipher_device.encrypt(block.data, block.data);
            out.write(block.data, cnt);
        }
        auto padbegin = in.gcount();
        auto padsize = cnt - padbegin;
        std::fill(block.data + padbegin, std::end(block.data), (std::uint8_t)padsize);
        cipher_device.encrypt(block.data, block.data);
        out.write(block.data, cnt);
    }
    void decrypt(std::istream& in, std::ostream& out)
    {
        constexpr auto cnt = Algorithm::block_size;
        memory_entity<Algorithm::block_size> block;
        while (true) {
            in.read(block.data, cnt);
            if (in.gcount() != cnt) throw std::runtime_error("decryption failed. invalid length");
            cipher_device.decrypt(block.data, block.data);
            if (!in.eof()) {
                out.write(block.data, cnt);
            } else {
                auto padsize = block[cnt - 1];
                if(check_pad(padsize, block[cnt - padsize]))
                    throw std::runtime_error("decryption failed. please check key or initial condition.");
                out.write(block.data, cnt - padsize);
                break;
            }
        }
    }
private:
    bool check_pad(size_t padsize, std::uint8_t* padbegin)
    {
        return padsize <= Algorithm::block_size &&
            std::count(padbegin, padbegin + padsize, (std::uint8_t)padsize) == padsize;
    }
    CipherMode<Algorithm> cipher_device;
};

}
