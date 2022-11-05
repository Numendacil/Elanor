#ifndef _CRICPP_ASF2_HPP_
#define _CRICPP_ASF2_HPP_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace cricpp
{

class ASF2View
{
protected:
	std::string_view _buffer;

	struct Header
	{
		uint8_t unknown1;
		uint8_t SizeBytes;
		uint8_t unknown2;
		uint8_t unknown3;
		uint32_t FileCount;	// LE
		uint16_t align;		// LE
		uint16_t key;		// LE
	};
	static_assert(std::is_standard_layout_v<Header> && std::is_trivial_v<Header>, "Not POD type");

	struct ByteArray { uint32_t start; uint32_t len; };

	Header _header;
	std::vector<uint16_t> _FileIds;		// LE
	std::vector<ByteArray> _files;

public:
	bool isParsed() const { return !this->_buffer.empty(); }
	size_t GetFileCount() const { return this->_files.size(); }

	std::string_view GetFileView(size_t idx) const
	{
		auto& arr = this->_files.at(idx);
		return this->_buffer.substr(arr.start, arr.len);
	}

	std::string GetFileCopy(size_t idx) const
	{
		auto& arr = this->_files.at(idx);
		return std::string(this->_buffer.substr(arr.start, arr.len));
	}

	uint16_t GetFileId(size_t idx) const { return this->_FileIds.at(idx); }

	uint16_t GetKey() const
	{
		return this->_header.key;
	}

	static ASF2View ParseASF2(std::string_view buffer);

};

}

#endif