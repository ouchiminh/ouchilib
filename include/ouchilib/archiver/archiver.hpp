#pragma once
#include <filesystem>
#include <vector>
#include <fstream>
#include "arcfile.hpp"

namespace ouchi::archive {
	class archiver {
		std::vector<file> list_;
		
		/// <summary>
		/// discard existing data, and read archive file header / reserve list_
		/// </summary>
		void init(std::istream & in) noexcept(false);
	public:
		/// <summary>
		/// add file to archive list.
		/// </summary>
		/// <param name="filepath">target</param>
		/// <returns>
		/// <para>false : if file does not exist, cannot be read, or failed for other reasons.</para>
		/// <para>true : if nothing happened(successed)</para>
		/// </returns>
		bool add_file(const std::filesystem::path & filepath) noexcept;
		void remove_file(const std::filesystem::path & filepath) noexcept;

		/// <summary>
		/// if fail, throw exception
		/// </summary>
		template<class OStream>
		void write(OStream & out) const;
		void write(const std::filesystem::path & filepath) const;

		/// <summary>load designated archive file.</summary>
		/// <param name = "filepath">path to the file will be loaded.</param>
		/// <returns>true:successed</returns>
		[[nodiscard]] bool load(const std::filesystem::path & filepath) noexcept;
		[[nodiscard]] bool load(std::istream & in) noexcept;

		[[nodiscard]] file & get(const std::filesystem::path & filepath);
		[[nodiscard]] const file & get(const std::filesystem::path & filepath) const;
	};
	template<class OStream>
	inline void archiver::write(OStream & out) const {
		using os = ouchi::ostream_traits<OStream>;
		auto filecnt = list_.size();
		os::write(out, static_cast<void*>(&filecnt), sizeof(filecnt));
		for (auto & i : list_) i.write_header(out);
		for (auto & i : list_) i.write_body(out);
	}
}
/** HEADER
 *	<file count><each header>......
 ** BODY
 *	<file body>.....
 **/
