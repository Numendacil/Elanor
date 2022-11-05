#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>

#include "utf.hpp"
#include "utils.hpp"

namespace cricpp
{

UTFView UTFView::ParseUTF(std::string_view buffer)
{
	using uchar_t = unsigned char;
	if (buffer.size() < sizeof(UTFView::Header) + 4)
		return {};

	if (buffer.substr(0, 4) != "@UTF")
		return {};

	UTFView view;
	size_t pos = 0;

	memcpy(&view._header, buffer.data() + 4, sizeof(UTFView::Header));
	UInt32BE(view._header.DataSize);
	UInt16BE(view._header.unknown);
	UInt16BE(view._header.ValueOffset);
	UInt32BE(view._header.StringOffset);
	UInt32BE(view._header.DataOffset);
	UInt32BE(view._header.NameOffset);
	UInt16BE(view._header.ElementCount);
	UInt16BE(view._header.ValueSize);
	UInt32BE(view._header.PageCount);
	pos += sizeof(UTFView::Header) - 4;

	if (buffer.size() < view._header.ValueOffset + 8)
		return {};
	if (buffer.size() < view._header.StringOffset + 8)
		return {};
	if (buffer.size() < view._header.DataOffset + 8)
		return {};

	view._buffer = buffer.substr(8);
	auto value_offset = view._header.ValueOffset;
	view._pages.reserve(view._header.ElementCount);
	auto first_pos = pos;
	for (size_t i = 0; i < view._header.PageCount; i++)
	{
		std::map<std::string, Element> page;
		pos = first_pos;
		for (size_t j = 0; j < view._header.ElementCount; j++)
		{
			Element element;
			auto type = static_cast<uchar_t>(view._buffer[pos]);
			pos++;
			std::string key;
			{
				element.StringOffset = ReadUInt32BE(view._buffer.data() + pos);
				pos += sizeof(uint32_t);

				key = view._GetString(view._header.StringOffset + element.StringOffset);
			}

			uchar_t method = type >> 5;
			type = type & 0x1F;	// b'00011111
			if (method > 0)
			{
				auto offset = (method == 1) ? pos : value_offset;
				const auto* org = reinterpret_cast<const uchar_t*>(view._buffer.data());

				switch (type) 
				{
				case ValueType::INT8:
					element.type = ValueType::INT8;
					element.value = static_cast<uint64_t>(
						*(org + offset)
					);
					offset++;
					break;
				case ValueType::UINT8:
					element.type = ValueType::UINT8;
					element.value = static_cast<uint64_t>(
						*(org + offset)
					);
					offset++;
					break;
				case ValueType::INT16:
					element.type = ValueType::INT16;
					element.value = static_cast<uint64_t>(ReadUInt16BE(org + offset));
					offset += 2;
					break;
				case ValueType::UINT16:
					element.type = ValueType::UINT16;
					element.value = static_cast<uint64_t>(ReadUInt16BE(org + offset));
					offset += 2;
					break;
				case ValueType::INT32:
					element.type = ValueType::INT32;
					element.value = static_cast<uint64_t>(ReadUInt32BE(org + offset));
					offset += 4;
					break;
				case ValueType::UINT32:
					element.type = ValueType::UINT32;
					element.value = static_cast<uint64_t>(ReadUInt32BE(org + offset));
					offset += 4;
					break;
				case ValueType::INT64:
					element.type = ValueType::INT64;
					element.value = ReadUInt64BE(org + offset);
					offset += 8;
					break;
				case ValueType::UINT64:
					element.type = ValueType::UINT64;
					element.value = ReadUInt64BE(org + offset);
					offset += 8;
					break;
				case ValueType::FLOAT:
					element.type = ValueType::FLOAT;
					element.value = static_cast<double>(ReadFloatBE(org + offset));
					offset += 4;
					break;
				case ValueType::DOUBLE:
					element.type = ValueType::DOUBLE;
					element.value = ReadDoubleBE(org + offset);
					offset += 8;
					break;
				case ValueType::STRING:
					element.type = ValueType::STRING;
					{
						uint32_t offset_str = ReadUInt32BE(org + offset);
						offset += 4;
						element.value = view._GetString(offset_str + view._header.DataOffset);
					}
					break;
				case ValueType::BYTES:
					element.type = ValueType::BYTES;
					{
						uint32_t offset_buffer = ReadUInt32BE(org + offset);
						offset += 4;
						uint32_t buffer_len = ReadUInt32BE(org + offset);
						offset += 4;
						element.value = ByteArray{
							offset_buffer + view._header.DataOffset,
							buffer_len
						};
					}
					break;
				default:
					throw std::runtime_error("Unknown type: " + std::to_string(type));
					break;
				}

				if (method == 1)
					pos = offset;
				else
					value_offset = offset;
			}
			page.emplace(std::move(key), std::move(element));
		}
		view._pages.push_back(std::move(page));
	}
	return view;
}

}