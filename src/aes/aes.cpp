#include <random>
#include <cstdlib>
#include <memory.h>
#include "ouchilib/aes/aes.hpp"
namespace {
constexpr auto NB = 4;
}
ouchi::aesKey::aesKey(std::function<std::uint32_t()>&& engine) : keyLength{ 32 } {
	std::random_device rd;
	for (auto & i : data.fours) i = (bool)engine ? engine() : rd();
}

uint32_t ouchi::aes::subWord(std::uint32_t in) const noexcept {
	uint32_t inw = in;
	unsigned char *cin = (unsigned char*)&inw;
	cin[0] = mSBox[cin[0]];
	cin[1] = mSBox[cin[1]];
	cin[2] = mSBox[cin[2]];
	cin[3] = mSBox[cin[3]];
	return (inw);
}

void ouchi::aes::expandKey() {
	/* FIPS 197  P.27 Appendix A.1 Rcon[i/Nk] */ //又は mulを使用する
	int Rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };
	std::uint32_t temp;

	memcpy(w, mKey.data.bytes, mKey.keyLength);
	for (size_t i = mKey.keyLength / 4; i<NB*(nr + 1); i++) {
		temp = w[i - 1];
		if ((i % (mKey.keyLength / 4)) == 0)
			temp = subWord(rotWord(temp)) ^ Rcon[i / (mKey.keyLength / 4) - 1];
		else if ((mKey.keyLength / 4) > 6 && (i % (mKey.keyLength / 4)) == 4)
			temp = subWord(temp);
		w[i] = w[i - (mKey.keyLength / 4)] ^ temp;
	}
}

void ouchi::aes::addRoundKey(void * data, int round)  const noexcept {
	auto ptr = (std::uint32_t*)data;
	for (int i = 0; i < 4; ++i) {
		ptr[i] ^= w[i + round * 4];
	}
}

void ouchi::aes::subBytes(void * data)  const noexcept {
	unsigned char *cb = (unsigned char*)data;
	for (int i = 0; i<16; i += 4)//理論的な意味から二重ループにしているが意味は無い
	{
		for (int j = 0; j<4; j++) {
			cb[i + j] = mSBox[cb[i + j]];
		}
	}
}

void ouchi::aes::invSubBytes(void * data)  const noexcept {
	unsigned char *cb = (unsigned char*)data;
	for (int i = 0; i<16; i += 4)//理論的な意味から二重ループにしているが意味は無い
	{
		for (int j = 0; j<4; j++) {
			cb[i + j] = mInvSBox[cb[i + j]];
		}
	}
}

void ouchi::aes::shiftRows(void * data) const noexcept {
	auto ptr{ (uint8_t*)data };
	block_16 cw(data);
	for (int i = 0; i<NB; i += 4) {
		int i4 = i * 4;
		for (int j = 1; j<4; j++) {
			cw.bytes[i4 + j + 0 * 4] = ptr[i4 + j + ((j + 0) & 3) * 4];
			cw.bytes[i4 + j + 1 * 4] = ptr[i4 + j + ((j + 1) & 3) * 4];
			cw.bytes[i4 + j + 2 * 4] = ptr[i4 + j + ((j + 2) & 3) * 4];
			cw.bytes[i4 + j + 3 * 4] = ptr[i4 + j + ((j + 3) & 3) * 4];
		}
	}
	memcpy(data, cw.bytes, sizeof(cw.bytes));
}

void ouchi::aes::invShiftRows(void * data) const noexcept {
	auto ptr{ (uint8_t*)data };
	block_16 cw(data);
	for (int i = 0; i<NB; i += 4) {
		int i4 = i * 4;
		for (int j = 1; j<4; j++) {
			cw.bytes[i4 + j + ((j + 0) & 3) * 4] = ptr[i4 + j + 0 * 4];
			cw.bytes[i4 + j + ((j + 1) & 3) * 4] = ptr[i4 + j + 1 * 4];
			cw.bytes[i4 + j + ((j + 2) & 3) * 4] = ptr[i4 + j + 2 * 4];
			cw.bytes[i4 + j + ((j + 3) & 3) * 4] = ptr[i4 + j + 3 * 4];
		}
	}
	memcpy(data, cw.bytes, sizeof(cw.bytes));
}

void ouchi::aes::mixColumns(uint32_t * data) const noexcept {
	using namespace ouchi;
	int i, i4, x;
	for (i = 0; i<NB; i++) {
		i4 = i * 4;
		x = mul(dataget(data, i4 + 0), 2) ^
			mul(dataget(data, i4 + 1), 3) ^
			mul(dataget(data, i4 + 2), 1) ^
			mul(dataget(data, i4 + 3), 1);
		x |= (mul(dataget(data, i4 + 1), 2) ^
			mul(dataget(data, i4 + 2), 3) ^
			mul(dataget(data, i4 + 3), 1) ^
			mul(dataget(data, i4 + 0), 1)) << 8;
		x |= (mul(dataget(data, i4 + 2), 2) ^
			mul(dataget(data, i4 + 3), 3) ^
			mul(dataget(data, i4 + 0), 1) ^
			mul(dataget(data, i4 + 1), 1)) << 16;
		x |= (mul(dataget(data, i4 + 3), 2) ^
			mul(dataget(data, i4 + 0), 3) ^
			mul(dataget(data, i4 + 1), 1) ^
			mul(dataget(data, i4 + 2), 1)) << 24;
		data[i] = x;
	}
}

void ouchi::aes::invMixColumns(uint32_t * data) const noexcept {
	using namespace ouchi;
	int i, i4, x;
	for (i = 0; i<NB; i++) {
		i4 = i * 4;
		x = mul(dataget(data, i4 + 0), 14) ^
			mul(dataget(data, i4 + 1), 11) ^
			mul(dataget(data, i4 + 2), 13) ^
			mul(dataget(data, i4 + 3), 9);
		x |= (mul(dataget(data, i4 + 1), 14) ^
			mul(dataget(data, i4 + 2), 11) ^
			mul(dataget(data, i4 + 3), 13) ^
			mul(dataget(data, i4 + 0), 9)) << 8;
		x |= (mul(dataget(data, i4 + 2), 14) ^
			mul(dataget(data, i4 + 3), 11) ^
			mul(dataget(data, i4 + 0), 13) ^
			mul(dataget(data, i4 + 1), 9)) << 16;
		x |= (mul(dataget(data, i4 + 3), 14) ^
			mul(dataget(data, i4 + 0), 11) ^
			mul(dataget(data, i4 + 1), 13) ^
			mul(dataget(data, i4 + 2), 9)) << 24;
		data[i] = x;
	}
}

ouchi::aes::aes() : mKey{}, nr{ mKey.keyLength / 4 + 6 } {}

ouchi::aes::aes(const aesKey & k, [[maybe_unused]] unsigned int threadnum) : mKey{ k }, nr{ mKey.keyLength / 4 + 6 } {}

ouchi::aes::aes(aesKey && k, [[maybe_unused]] unsigned int threadnum) : mKey{ k }, nr{ mKey.keyLength / 4 + 6 } {}

void ouchi::aes::setKey(const aesKey & k) {
	if (k.keyLength % 8 == 0 && k.keyLength / 8 < 1) throw std::runtime_error("invalid keyLength");
	mKey = k;
	nr = mKey.keyLength / 4 + 6;
}

const ouchi::aesKey & ouchi::aes::getKey() const noexcept {
	return mKey;
}

std::vector<uint8_t> ouchi::padData(const void * data, const size_t sizeInByte) {
	const std::uint8_t rest = (sizeInByte % 16);
	const auto * ptr = (const uint8_t*)data;
	std::vector<uint8_t> result;
	for (size_t i = 0; i < sizeInByte; i++) {
		result.push_back(*ptr);
		ptr++;
	}
	for (int i = 0; i < 16 - rest; i++) result.push_back(16 - rest);
	return result;
}

ouchi::block_16 ouchi::makeIV() {
	std::random_device rd;
	ouchi::block_16 iv;
	for (auto & i : iv.bytes) i ^= rd();
	return iv;
}

void ouchi::add(block_16 & a, const block_16 & b) {
	for (int i = 0; i < 16; i++) {
		a.bytes[i] ^= b.bytes[i];
	}
}

void ouchi::add(void * a, const void * b, const size_t sizeInByte) {
	auto ptr = (uint8_t*)a;
	const auto * ptr2 = (uint8_t*)b;
	for (unsigned i = 0; i < sizeInByte; ++i) ptr[i] ^= ptr2[i];

}

std::uint32_t ouchi::rotWord(std::uint32_t in) {
	uint32_t inw = in, inw2 = 0;
	unsigned char *cin = (unsigned char*)&inw;
	unsigned char *cin2 = (unsigned char*)&inw2;
	cin2[0] = cin[1];
	cin2[1] = cin[2];
	cin2[2] = cin[3];
	cin2[3] = cin[0];
	return(inw2);
}

int ouchi::mul(int dt, int n) {
	int x = 0;
	for (int i = 8; i>0; i >>= 1) {
		x <<= 1;
		if (x & 0x100)
			x = (x ^ 0x1b) & 0xff;
		if ((n & i))
			x ^= dt;
	}
	return(x);
}

int ouchi::dataget(void * data, int n) {
	return (((unsigned char*)data)[n]);
}
