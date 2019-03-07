﻿#ifndef AES_HPP
#define AES_HPP

#include <cstdint>
#include <vector>
#include <array>
#include <functional>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <memory.h>
#include <cstring>

namespace ouchi {
	union kc {
		uint8_t bytes[32];
		std::uint32_t fours[8];
	};
	template<size_t size = 16>
	union block {
		uint8_t bytes[size];
		uint32_t fours[size/4];

		block(void * data);
		block();

		block & operator=(const void * data);
		bool operator==(const block<size> & obj);
	};

	template<size_t size>
	ouchi::block<size>::block(void * data) {
		memcpy(bytes, data, size);
	}

	template<size_t size>
	ouchi::block<size>::block() : bytes{} {}

	template<size_t size>
	ouchi::block<size> & ouchi::block<size>::operator=(const void * data) {
		memcpy(bytes, data, size);
		return *this;
	}

	template<size_t size>
	bool ouchi::block<size>::operator==(const block<size> & obj) {
		for (unsigned i = 0; i < size; i++)
			if (obj.bytes[i] != bytes[i]) return false;
		return true;
	}

	using block_16 = block<16>;

	enum class CIPHERMODE { ECB, CBC };
	struct aesKey {
		kc data;
		size_t keyLength;
		aesKey(std::function<std::uint32_t()> && engine = std::function<std::uint32_t()>{});
		template<size_t sizeInByte>
		aesKey(std::array<std::uint8_t, sizeInByte> key);
	};
#pragma region utilFuncDecl
	std::vector<uint8_t> padData(const void * data, const size_t sizeInByte);
	[[nodiscard]] block_16 makeIV();
	void add(block_16 & a, const block_16 & b);
	void add(void * a, const void * b, const size_t sizeInByte = 16);

	std::uint32_t rotWord(std::uint32_t in);
	int mul(int dt, int n);
	int dataget(void * data, int n);
#pragma endregion

	class aes {
		aesKey mKey;		// encryption key
		size_t nr;		// round cnt
		std::uint32_t w[60];	// expanded key
		mutable block_16 prevBlock;		// use when CBC
		static inline constexpr unsigned char mSBox[256] = {
			0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
			0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
			0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
			0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
			0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
			0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
			0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
			0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
			0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
			0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
			0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
			0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
			0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
			0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
			0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
			0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
		};
		static inline constexpr unsigned char mInvSBox[256] = {
			0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
			0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
			0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
			0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
			0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
			0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
			0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
			0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
			0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
			0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
			0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
			0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
			0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
			0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
			0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
			0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
		};
	private:
		std::uint32_t subWord(std::uint32_t in) const noexcept;
		void expandKey();
		void addRoundKey(void * data, int round) const noexcept;

		void subBytes(void * data) const noexcept;
		void invSubBytes(void * data) const noexcept;

		void shiftRows(void * data) const noexcept;
		void invShiftRows(void * data) const noexcept;

		void mixColumns(uint32_t * data) const noexcept;
		void invMixColumns(uint32_t *data) const noexcept;

		template<CIPHERMODE mode>
		void encryptBlock(void * data) const noexcept;
		template<CIPHERMODE mode>
		void decryptBlock(void * data) const noexcept;
	public:
		aes();
		aes(const aesKey & k, unsigned int threadnum = 0);
		aes(aesKey && k, unsigned int threadnum = 0);

		void setKey(const aesKey & k);
		const aesKey & getKey() const noexcept;

		template<CIPHERMODE mode = CIPHERMODE::CBC>
		std::vector<uint8_t> encrypt(const void * data, const size_t sizeInByte) ;
		template<CIPHERMODE mode = CIPHERMODE::CBC>
		void encrypt(std::istream & in, std::ostream & out) ;

		template<CIPHERMODE mode = CIPHERMODE::CBC>
		std::vector<uint8_t> decrypt(const void * data, const size_t sizeInByte) noexcept(false);
		template<CIPHERMODE mode = CIPHERMODE::CBC>
		void decrypt(std::istream & in, std::ostream & out) noexcept(false);
	};

#pragma region definition

	template<size_t sizeInByte>
	inline aesKey::aesKey(std::array<std::uint8_t, sizeInByte> key) : data{ { 0 } } {
		static_assert(sizeInByte == 16 || sizeInByte == 24 || sizeInByte == 32, "key size must be 128/192/256 bit.");
		keyLength = sizeInByte;
		memcpy(data.bytes, &key[0], sizeInByte);
	}


	template<CIPHERMODE mode>
	inline void aes::encryptBlock(void * data) const noexcept {
		unsigned i = 0;
		if constexpr (CIPHERMODE::CBC == mode) add(data, prevBlock.bytes);
		addRoundKey(data, 0);
		for (i = 1; i < nr; ++i) {
			subBytes(data);
			shiftRows(data);
			mixColumns((uint32_t*)data);
			addRoundKey(data, i);
		}
		subBytes(data);
		shiftRows(data);
		addRoundKey(data, i);
		if constexpr (CIPHERMODE::CBC == mode) prevBlock = data;
	}

	template<CIPHERMODE mode>
	inline void aes::decryptBlock(void * data) const noexcept {
		size_t i;
		block_16 pbTmp(data);
		addRoundKey(data, (int)nr);
		for (i = nr - 1; i > 0; i--) {
			invShiftRows(data);
			invSubBytes(data);
			addRoundKey(data, (int)i);
			invMixColumns((uint32_t*)data);
		}

		invShiftRows(data);
		invSubBytes(data);
		addRoundKey(data, 0);
		if constexpr (CIPHERMODE::CBC == mode) {
			add(data, prevBlock.bytes);
			prevBlock = pbTmp;
		}
	}

	template<CIPHERMODE mode>
	[[nodiscard]] inline std::vector<uint8_t> aes::encrypt(const void * data, const size_t sizeInByte)  {
		std::vector<uint8_t> result;
		expandKey();
		if constexpr (CIPHERMODE::CBC == mode) {
			prevBlock = makeIV();
			// iv入力
			for (const auto & i : prevBlock.bytes) result.push_back(i);
		}
		// 詰め物
		for (const auto & i : padData(data, sizeInByte)) result.push_back(i);
		for (size_t i = mode == CIPHERMODE::CBC ? 16 : 0; i < result.size(); i += 16) {
			encryptBlock<mode>(&result.at(i));
		}

		return result;
	}

	template<CIPHERMODE mode>
	inline void aes::encrypt(std::istream & in, std::ostream & out)  {
		block_16 b;
		bool bend{false};
		expandKey();
		if(!in || !out) throw std::runtime_error("file stream error");
		if constexpr (CIPHERMODE::CBC == mode) {
			prevBlock = makeIV();
			out.write((char*)prevBlock.bytes, 16);
		}
		while(!bend){
			in.read((char*)b.bytes, 16);
			if(in.eof()){
				auto rcnt{in.gcount()};
				auto padded{padData(b.bytes, rcnt)};
				b = (void*)padded.data();
				encryptBlock<mode>(b.bytes);
				out.write((char*)b.bytes, 16);
				if(padded.size() == 32)
					b = (void*)&padded.at(16);
				else break;
				bend = true;
			}
			encryptBlock<mode>(b.bytes);
			out.write((char*)b.bytes, 16);
			memset(b.bytes, 0, 16);
		}
	}

	template<CIPHERMODE mode>
	[[nodiscard]] inline std::vector<uint8_t> aes::decrypt(const void * data, const size_t sizeInByte)  noexcept(false) {
		std::vector<uint8_t> result;
		expandKey();
		if (sizeInByte % 16 != 0 || sizeInByte == 0) throw std::runtime_error("invalid data length");
		if constexpr (CIPHERMODE::CBC == mode) {
			// iv避難
			prevBlock = data;
			// iv消去
			data = (uint8_t*)data + 16;
		}
		for (unsigned i = 0; i < sizeInByte - (CIPHERMODE::CBC == mode ? 16 : 0); ++i) result.push_back(*((uint8_t*)data + i));
		for (size_t i = 0; i < result.size(); i += 16) {
			decryptBlock<mode>(&result.at(i));
		}
		auto delCnt = result.back();
		if (delCnt > result.size()) throw std::runtime_error("corrupted data");
		for (unsigned i = 0; i < delCnt; i++) result.pop_back();
		return result;
	}

	template<CIPHERMODE mode>
	inline void aes::decrypt(std::istream & in, std::ostream & out)  noexcept(false) {
		block_16 b;
		size_t size;
		if(!in || !out) throw std::runtime_error("file stream error");
		in.seekg(0, std::ios::end);
		size = in.tellg() - std::streampos(16);
		in.seekg(0, std::ios::beg);
		expandKey();
		if constexpr (CIPHERMODE::CBC == mode) {
			in.read((char*)&b.bytes, sizeof(b.bytes));
			prevBlock = b;
		}
		for (size_t i = 0; i < size; i+=16) {
			in.read((char*)&b.bytes, sizeof(b.bytes));
			decryptBlock<mode>(b.bytes);
			if (i == size - 16 && static_cast<int>(16 - b.bytes[15]) < 0) throw std::runtime_error("corrupted data");
			out.write((char*)b.bytes, (i == size - 16) ? 16 - b.bytes[15] : 16);
		}
	}

#pragma endregion
}

#endif
