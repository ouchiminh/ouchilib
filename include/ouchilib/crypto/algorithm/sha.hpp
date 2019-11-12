#pragma once
#include <cstring>
#include <cstdint>
#include <string_view>
#include <utility>
#include "../common.hpp"
#include "ouchilib/utl/multiitr.hpp"

namespace ouchi::crypto {

namespace detail {

template<class Int, std::enable_if_t<std::is_integral_v<Int>&& std::is_unsigned_v<Int>>* = nullptr>
inline constexpr Int ch(Int x, Int y, Int z) noexcept
{
    return (x & y) ^ ((~x) & z);
}

template<class Int, std::enable_if_t<std::is_integral_v<Int>&& std::is_unsigned_v<Int>>* = nullptr>
inline constexpr Int maj(Int x, Int y, Int z) noexcept
{
    return (x & y) ^ (x & z) ^ (y & z);
}

// sha-224, sha-256
inline constexpr std::uint32_t sha_f0(std::uint32_t x) noexcept
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
inline constexpr std::uint32_t sha_f1(std::uint32_t x) noexcept
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}
inline constexpr std::uint32_t sha_f2(std::uint32_t x) noexcept
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}
inline constexpr std::uint32_t sha_f3(std::uint32_t x) noexcept
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

// sha-384, sha512
inline constexpr std::uint64_t sha_f0(std::uint64_t x) noexcept
{
    return rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39);
}
inline constexpr std::uint64_t sha_f1(std::uint64_t x) noexcept
{
    return rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41);
}
inline constexpr std::uint64_t sha_f2(std::uint64_t x) noexcept
{
    return rotr(x, 1) ^ rotr(x, 8) ^ (x >> 7);
}
inline constexpr std::uint64_t sha_f3(std::uint64_t x) noexcept
{
    return rotr(x, 19) ^ rotr(x, 61) ^ (x >> 6);
}

// constants
template<class Int>
inline constexpr void* sha_constants = nullptr;

// constants for sha-224, 256
template<>
inline constexpr std::uint32_t sha_constants<std::uint32_t>[64] = {
0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
// constants for sha-384, 512
template<>
inline constexpr std::uint64_t sha_constants<std::uint64_t>[80] = {
0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

template<class Int>
inline constexpr void* initial_hash_value = nullptr;

template<>
inline constexpr std::uint32_t initial_hash_value<std::uint32_t>[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
template<>
inline constexpr std::uint64_t initial_hash_value<std::uint64_t>[8] = {
        0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
        0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

inline constexpr int v[] = { 1, 2, 3, 4 };

} // namespace detail

template<int Len>
class sha {
    static constexpr unsigned last_block_m_length = (1024 - 128) / 8 / (Len <= 256 ? 2 : 1);
    static constexpr unsigned block_length = 1024 / 8 / (Len <= 256 ? 2 : 1);
    using elm_type = std::conditional_t<(Len > 256), std::uint64_t, std::uint32_t>;
    elm_type h_[8];
    size_t length_;
    std::uint8_t buffer_[block_length];
public:
    sha()
        : h_{}
        , length_{0}
        , buffer_{}
    {
        for (auto [h, ih] : ouchi::multiitr{ h_, detail::initial_hash_value<elm_type> }) {
            h = ih;
        }
    }
    void update(std::string_view message) noexcept
    {
        update(message.data(), message.size());
    }
    void update(const void* message, size_t size) noexcept
    {
        constexpr size_t lidx = block_length - 1;
        auto* ptr = reinterpret_cast<const std::uint8_t*>(message);
        while (size) {
            buffer_[length_ & lidx] = *ptr++;
            --size;
            ++length_;
            if ((length_ & lidx) == 0) {
                process_block(buffer_);
            }
        }
    }
    memory_entity<Len/8> finalize() noexcept
    {
        constexpr size_t lidx = block_length - 1;
        buffer_[length_ & lidx] = 0x80;
        auto tmplen = length_ + 1;
        if ((tmplen & lidx) == 0) {
            process_block(buffer_);
        }
        while ((tmplen & lidx) % last_block_m_length != 0 || !(tmplen & lidx)) {
            buffer_[tmplen++ & lidx] = 0;
            if ((tmplen & lidx) == 0) {
                process_block(buffer_);
            }
        }
        if constexpr (sizeof(elm_type) == sizeof(std::uint64_t)) {
            detail::unpack(0, buffer_ + last_block_m_length);
            detail::unpack(length_ * 8, buffer_ + last_block_m_length + sizeof(elm_type));
        } else {
            detail::unpack(length_ * 8, buffer_ + last_block_m_length);
        }
        process_block(buffer_);
        secure_memset(buffer_, 0);
        memory_entity<Len/8> ret{};
        for (auto i = 0u; i < sizeof(h_) / sizeof(*h_); ++i) {
            detail::unpack(h_[i], ret.data + (i * sizeof(elm_type)));
        }
        return ret;
    }

private:
    void process_block(void* block) noexcept
    {
        constexpr size_t w_size = Len > 256 ? 80 : 64;
        elm_type w[w_size] = {};
        auto* ptr = reinterpret_cast<const std::uint8_t*>(block);
        for (auto i = 0u; i < 16; ++i) {
            w[i] = detail::pack<elm_type>(ptr + (i * sizeof(elm_type)));
        }
        for (auto i = 16u; i < w_size; ++i) {
            w[i] = detail::sha_f3(w[i - 2]) + w[i - 7] + detail::sha_f2(w[i - 15]) + w[i - 16];
        }
        elm_type a{ h_[0] }, b{ h_[1] }, c{ h_[2] }, d{ h_[3] },
            e{ h_[4] }, f{ h_[5] }, g{ h_[6] }, h{ h_[7] };
        for (auto i = 0u; i < w_size; ++i) {
            auto t1 = h + detail::sha_f1(e) + detail::ch(e, f, g) +
                detail::sha_constants<elm_type>[i] + w[i];
            auto t2 = detail::sha_f0(a) + detail::maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        h_[0] = a + h_[0];
        h_[1] = b + h_[1];
        h_[2] = c + h_[2];
        h_[3] = d + h_[3];
        h_[4] = e + h_[4];
        h_[5] = f + h_[5];
        h_[6] = g + h_[6];
        h_[7] = h + h_[7];
        //std::memset(block, 0, block_length);
    }
};

using sha512 = sha<512>;
using sha256 = sha<256>;

}
namespace ouchi {

inline namespace literals {
inline namespace crypto_literals {

crypto::memory_entity<64> operator""_sha512(const char* str, [[maybe_unused]] size_t size)
{
    crypto::sha512 hash;
    hash.update(str);
    return hash.finalize();
}

crypto::memory_entity<32> operator""_sha256(const char* str, [[maybe_unused]] size_t size)
{
    crypto::sha256 hash;
    hash.update(str);
    return hash.finalize();
}

} // namespace crypto_literals
}

}
