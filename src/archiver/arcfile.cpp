#include <memory.h>
#include "ouchilib/archiver/arcfile.hpp"

void ouchi::archive::file::discard() noexcept {
	size_ = 0;
	if (body_) delete[] body_;
	body_ = nullptr;
	filepath_.clear();
}

ouchi::archive::file::file(std::filesystem::path && filepath) noexcept(false) : body_( nullptr ), size_{ 0 } { load_from_file(filepath); }

ouchi::archive::file::file(const std::filesystem::path & filepath) : body_( nullptr ), size_{ 0 } { load_from_file(filepath); }

ouchi::archive::file::file(const std::string & filepath, size_t size) : body_( nullptr ), size_( 0 ) { 
	register_archived_info(std::filesystem::path(filepath), size); 
}

ouchi::archive::file::file() noexcept : body_{ nullptr }, size_{ 0 } {}

ouchi::archive::file::file(file && f) : body_{ f.body_ }, size_{ f.size_ }, filepath_{ f.filepath_ } {
	f.body_ = nullptr;
	f.size_ = 0;
	f.filepath_.clear();
}

ouchi::archive::file::~file() {
	discard();
}

inline void ouchi::archive::file::load_from_file(const std::filesystem::path & filepath) {
	namespace fs = std::filesystem;
	if (!fs::exists(filepath)) throw std::runtime_error("file not found.");
	discard();
	std::ifstream in;
	filepath_ = filepath;
	in.open(filepath, std::ios::binary | std::ios::in);
	try {
		size_ = fs::file_size(filepath);
		body_ = new std::int8_t[size_ + 1];
		in.read(static_cast<char*>(body_), size_);
	} catch (std::bad_alloc & e) {
		throw std::runtime_error(e.what());
	} catch (std::exception & e) { throw std::runtime_error(e.what()); }
}

/// <summary>
/// register header info before reading file body.
/// </summary>
/// <param name="filepath">filepath. could be an identifier</param>
/// <param name="filesize">filesize. this param is used when load_from_archive</param>

inline void ouchi::archive::file::register_archived_info(const std::filesystem::path & filepath, size_t filesize) noexcept {
	discard();
	filepath_ = filepath;
	size_ = filesize;
}

bool ouchi::archive::file::is_valid() const noexcept {
	return body_ != nullptr &&
		size_ != 0 &&
		filepath_.empty() ?
		false :
		true;
}

const std::filesystem::path ouchi::archive::file::get_filepath() const noexcept {
	return filepath_;
}

void * ouchi::archive::file::get_body() noexcept {
	return body_;
}

void const * ouchi::archive::file::get_body() const noexcept {
	return body_;
}

size_t ouchi::archive::file::size() const noexcept {
	return size_;
}

bool ouchi::archive::file::operator==(const std::filesystem::path & p) const noexcept {
	namespace fs = std::filesystem;
	const auto cur = fs::current_path();
	return p == filepath_;
}

ouchi::archive::file & ouchi::archive::file::operator=(file && f) {
	body_ = f.body_;
	size_ = f.size_;
	filepath_ = f.filepath_;

	f.body_ = nullptr;
	f.size_ = 0;
	f.filepath_.clear();
	return *this;
}
