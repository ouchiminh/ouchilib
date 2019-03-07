#pragma once
#include <vector>
#include <istream>
#include <ostream>
#include <cstdint>
#include "aes.hpp"
#include "SHA256.hpp"

namespace ouchi {
	class aes_utl {
		aes encoder_;
	public:
		aes_utl() = default;
		aes_utl(aes && encoder);
		aes_utl(const aes & encoder);

		void set_encoder(aes && encoder);
		aes & get_encoder() noexcept;
		const aes & get_encoder() const noexcept;

		[[nodiscard]] std::vector<std::uint8_t> decrypt(const std::string & filename);
		[[nodiscard]] std::vector<std::uint8_t> decrypt(std::istream & in);
		void decrypt(std::istream & in, std::ostream & out);
		
		void encrypt(std::istream & in, std::ostream & out);
	};
	
	inline std::vector<std::uint8_t> aes_utl::decrypt(std::istream & in) {
        std::streamoff size;
        {
            auto original = in.tellg();
            in.seekg(0, std::ios_base::end);
            size = in.tellg() - original;
            in.seekg(original, std::ios_base::beg);
        }
		void * data = new uint8_t[size];
		SHA256 hash;
		block<32> newb, orgb;

        in.read(reinterpret_cast<char*>(data), size);
		auto result = encoder_.decrypt(data, size);
		delete[] (uint8_t*)data;
		// hashの分も考えてデータが短ければ例外投げて終わろう。
		if (result.size() <= 32) throw std::runtime_error("invalid length");
		orgb = result.data() + (result.size() - 32);
		hash.Push(result.data(), (u_32)result.size() - 32);
		hash.Final((unsigned char*)(void*)newb.bytes);

		for (int i = 0; i < 32; i++) result.pop_back();
		
		if (orgb == newb) return std::forward<std::vector<std::uint8_t>>(result);
		throw std::runtime_error("hash incorrenct. data may be corrupted.");
	}

	inline void aes_utl::decrypt(std::istream & in, std::ostream & out) {
		std::vector<std::uint8_t> result = decrypt(in);
        out.write(reinterpret_cast<char*>(result.data()), result.size());
	}

	inline void aes_utl::encrypt(std::istream & in, std::ostream & out) {
        std::streamoff size;
        {
            auto original = in.tellg();
            in.seekg(0, std::ios_base::end);
            size = in.tellg() - original;
            in.seekg(original, std::ios_base::beg);
        }
		SHA256 hash;
		void * data;
		data = new std::uint8_t[size + 32];

        in.read(reinterpret_cast<char*>(data), size);
		hash.Push(static_cast<unsigned char*>(data), static_cast<u_32>(size));
		hash.Final(static_cast<unsigned char*>(data) + size);
		auto result = encoder_.encrypt(data, size + 32);

		delete[] data;
        out.write(reinterpret_cast<char*>(result.data()), result.size());
	}
}
