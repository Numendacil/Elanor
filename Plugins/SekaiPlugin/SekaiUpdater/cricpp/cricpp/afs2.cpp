#include "afs2.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include "utils.hpp"

namespace cricpp
{

ASF2View ASF2View::ParseASF2(std::string_view buffer)
{
	if (buffer.size() < sizeof(Header) + 4) return {};

	if (buffer.substr(0, 4) != "AFS2") return {};

	ASF2View view;
	auto pos = buffer.data() + 4;

	memcpy(&view._header, pos, sizeof(Header));
	UInt32LE(view._header.FileCount);
	UInt16LE(view._header.align);
	UInt16LE(view._header.key);
	pos += sizeof(Header);

	view._FileIds.resize(view._header.FileCount);
	memcpy(view._FileIds.data(), pos, sizeof(uint16_t) * view._header.FileCount);
	for (auto& i : view._FileIds)
		UInt16LE(i);
	pos += sizeof(uint16_t) * view._header.FileCount;

	int64_t start{};
	{
		if (view._header.SizeBytes == 2)
		{
			start = ReadUInt16LE(pos);
			pos += 2;
		}
		else if (view._header.SizeBytes == 4)
		{
			start = ReadUInt32LE(pos);
			pos += 4;
		}
		else
			throw std::runtime_error("Unsupported SizeBytes: " 
					+ std::to_string(view._header.SizeBytes));
		auto mod = start % view._header.align;
		if (mod != 0) start += view._header.align - mod;
	}

	view._files.reserve(view._header.FileCount);
	for (size_t i = 0; i < view._header.FileCount; i++)
	{
		int64_t end{};
		{
			if (view._header.SizeBytes == 2)
			{
				end = ReadUInt16LE(pos);
				pos += 2;
			}
			else if (view._header.SizeBytes == 4)
			{
				end = ReadUInt32LE(pos);
				pos += 4;
			}
			else
				throw std::runtime_error("Unsupported SizeBytes: " 
						+ std::to_string(view._header.SizeBytes));
		}
		view._files.emplace_back(start, end - start);

		start = end;
		auto mod = start % view._header.align;
		if (mod != 0) start += view._header.align - mod;
	}

	view._buffer = buffer;

	return view;
}

}