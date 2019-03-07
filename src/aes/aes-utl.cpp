#include <fstream>
#include <stdexcept>
#include "ouchilib/aes/aes-utl.hpp"

ouchi::aes_utl::aes_utl(aes && encoder) : encoder_{encoder} {}

ouchi::aes_utl::aes_utl(const aes & encoder) : encoder_{encoder} {}

void ouchi::aes_utl::set_encoder(aes && encoder) {
	encoder_ = std::move(encoder);
}

ouchi::aes & ouchi::aes_utl::get_encoder() noexcept {
	return encoder_;
}

const ouchi::aes & ouchi::aes_utl::get_encoder() const noexcept {
	return encoder_;
}

std::vector<std::uint8_t> ouchi::aes_utl::decrypt(const std::string & filename) {
	std::ifstream in;
	in.open(filename, std::ios::binary | std::ios::in);
	if (!in) throw std::runtime_error("cannot open file");
	return this->decrypt(in);
}

