#pragma once
#include <stdexcept>
#include <filesystem>
#include <string>
#include <cstddef>
#include <fstream>
#include <utility>
#include "../utl/stream-traits.hpp"

namespace ouchi::archive {
	class file {
		void * body_;	// if invalid, nullptr
		size_t size_;	// if invalid, 0
		std::filesystem::path filepath_;

		void discard() noexcept;
	public:
		/// <summary>
		/// construct & load content from normal file.
		/// </summary>
		/// <param name="filepath">path to normal file NOT ARCHIVE HEADER INFO</param>
		/// <returns></returns>
		file(std::filesystem::path && filepath) noexcept(false);
		file(const std::filesystem::path & filepath);
		file(const std::string & filepath, size_t size);
		file() noexcept;			// initialize as invalid object
		file(const file &) = delete;
		file(file && f);
		~file();

		void load_from_file(const std::filesystem::path & filepath);
		/// <summary>
		/// register header info before reading file body.
		/// </summary>
		/// <param name="filepath">filepath. could be an identifier</param>
		/// <param name="filesize">filesize. this param is used when load_from_archive</param>
		void register_archived_info(const std::filesystem::path & filepath, size_t filesize) noexcept;

		/// <summary>
		/// load file body from archive file from in's position.
		/// </summary>
		/// <param name="in">source</param>
		template<class IStream>
		void load_from_archive(IStream & in) noexcept(false);

		bool is_valid() const noexcept;
		const std::filesystem::path get_filepath() const noexcept;
		void * get_body() noexcept;
		void const * get_body() const noexcept;
		size_t size() const noexcept;

		template<class OStream>
		void write_header(OStream & out) const noexcept(false);
		template<class OStream>
		void write_body(OStream & out) const noexcept(false);

		bool operator==(const std::filesystem::path & p) const noexcept;
		file & operator=(file && f);
	};

	template<class IStream>
	inline void file::load_from_archive(IStream & in) noexcept(false) {
		using is = ouchi::istream_traits<IStream>;
		if (filepath_.empty() || size_ == 0) throw std::logic_error("invalid pre-condition.");
		body_ = new std::int8_t[size_];
		is::read(in, body_, size_);
	}

	template<class OStream>
	inline void file::write_header(OStream & out) const noexcept(false) {
		namespace fs = std::filesystem;
		using is = ostream_traits<OStream>;
		if (!is_valid()) throw(std::runtime_error("invalid data"));
		is::write(out, (void*)filepath_.string<char>().c_str(), filepath_.string<char>().size() + 1);
		is::write(out, (void*)&size_, sizeof(size_));
	}

	template<class OStream>
	inline void file::write_body(OStream & out) const noexcept(false) {
		using is = ostream_traits<OStream>;
		if (!is_valid()) throw std::runtime_error("invalid data");
		is::write(out, body_, size_);
	}

	class file_view {
		void const * body_;
		size_t size_;
		std::filesystem::path filepath_;
	public:
		file_view(const file & f) : body_{ f.get_body() }, size_{ f.size() }, filepath_{ f.get_filepath() } {}
		file_view() : body_{ nullptr }, size_{ 0 } {}

		bool is_valid() const noexcept { return body_ != nullptr && size_; }
		void const * get_body() const noexcept { return body_; }
		size_t size() const noexcept { return size_; }
		
		file_view & operator=(const file & f) {
			body_ = f.get_body();
			size_ = f.size();
			filepath_ = f.get_filepath();
			return *this;
		}
	};
}
