#ifndef _CRICPP_UTF_CPP_
#define _CRICPP_UTF_CPP_

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace cricpp
{

struct UTFTypeError : public std::runtime_error 
{
	using runtime_error::runtime_error;
};


struct UTFView
{
public:
	enum ValueType : uint8_t
	{
		INT8 = 0x10,
		UINT8 = 0x11,
		INT16 = 0x12,
		UINT16 = 0x13,
		INT32 = 0x14,
		UINT32 = 0x15,
		INT64 = 0x16,
		UINT64 = 0x17,
		FLOAT = 0x18,
		DOUBLE = 0x19,
		STRING = 0x1A,
		BYTES = 0x1B
	};

protected:
	std::string_view _buffer;

	struct Header
	{
		uint32_t DataSize;		// BE
		uint16_t unknown;		// BE
		uint16_t ValueOffset;		// BE
		uint32_t StringOffset;		// BE
		uint32_t DataOffset;		// BE
		uint32_t NameOffset;		// BE
		uint16_t ElementCount;		// BE
		uint16_t ValueSize;		// BE
		uint32_t PageCount;		// BE
	};
	static_assert(std::is_standard_layout_v<Header> && std::is_trivial_v<Header>, "Not POD type");

	struct ByteArray { uint32_t start; uint32_t len; };

	struct Element
	{
		ValueType type;		// BE
		uint32_t StringOffset;	// BE

		std::variant<
			uint64_t, 
			double, 
			std::string,
			ByteArray
		> value = uint64_t{};
	};

	Header _header;
	std::vector<std::map<std::string, Element>> _pages;
	
	size_t _FindStringEnd(size_t offset) const
	{
		while(offset < this->_buffer.size())
		{
			if (this->_buffer[offset] == '\0')
				return offset;
			offset++;
		}
		return this->_buffer.size() - 1;
	}

	std::string _GetString(size_t offset) const
	{
		auto end = _FindStringEnd(offset);
		return std::string(this->_buffer.substr(offset, end - offset));
	}

public:
	bool isParsed() const { return !this->_buffer.empty(); }

	std::string GetName() const
	{
		return _GetString(this->_header.NameOffset + this->_header.StringOffset);
	}

	size_t GetPageCount() const { return this->_pages.size(); }
	std::vector<std::string> GetElementKeys(size_t page) const 
	{
		std::vector<std::string> keys;
		keys.reserve(this->_pages.at(page).size());
		for (const auto& p : this->_pages.at(page))
			keys.push_back(p.first);
		return keys;
	}

	ValueType GetValueType(size_t page, const std::string& key) const
	{
		return this->_pages.at(page).at(key).type;
	}

	uint64_t GetUIntValue(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type < 0x18))
			throw UTFTypeError("value is not an integer");
		return std::get<uint64_t>(element.value);
	}

	uint8_t GetUInt8Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == UINT8))
			throw UTFTypeError("value is not of type uint8");
		return static_cast<uint8_t>(std::get<uint64_t>(element.value));
	}
	
	uint16_t GetUInt16Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == UINT16))
			throw UTFTypeError("value is not of type uint16");
		return static_cast<uint16_t>(std::get<uint64_t>(element.value));
	}
	
	uint32_t GetUInt32Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == UINT32))
			throw UTFTypeError("value is not of type uint32");
		return static_cast<uint32_t>(std::get<uint64_t>(element.value));
	}
	
	uint64_t GetUInt64Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == UINT64))
			throw UTFTypeError("value is not of type uint64");
		return std::get<uint64_t>(element.value);
	}

	int64_t GetIntValue(size_t page, const std::string& key) const
	{
		return static_cast<int64_t>(GetUIntValue(page, key));
	}

	int8_t GetInt8Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == INT8))
			throw UTFTypeError("value is not of type int8");
		return static_cast<int8_t>(std::get<uint64_t>(element.value));
	}
	
	int16_t GetInt16Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == INT16))
			throw UTFTypeError("value is not of type int16");
		return static_cast<int16_t>(std::get<uint64_t>(element.value));
	}
	
	int32_t GetInt32Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == INT32))
			throw UTFTypeError("value is not of type int32");
		return static_cast<int32_t>(std::get<uint64_t>(element.value));
	}
	
	int64_t GetInt64Value(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == INT64))
			throw UTFTypeError("value is not of type int64");
		return static_cast<int64_t>(std::get<uint64_t>(element.value));
	}

	double GetFloatedValue(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == FLOAT || element.type == DOUBLE))
			throw UTFTypeError("value is not a floated type");
		return std::get<double>(element.value);
	}

	float GetFloatValue(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == FLOAT))
			throw UTFTypeError("value is not a float type");
		return static_cast<float>(std::get<double>(element.value));
	}

	double GetDoubleValue(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == DOUBLE))
			throw UTFTypeError("value is not a double type");
		return std::get<double>(element.value);
	}

	std::string GetString(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == STRING))
			throw UTFTypeError("value is not a string");
		return std::get<std::string>(element.value);
	}

	std::string_view GetBytesView(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == BYTES))
			throw UTFTypeError("value is not a byte array");
		auto& arr = std::get<ByteArray>(element.value);
		return this->_buffer.substr(arr.start, arr.len);
	}

	std::string GetBytesCopy(size_t page, const std::string& key) const
	{
		auto& element = this->_pages.at(page).at(key);
		if (!(element.type == BYTES))
			throw UTFTypeError("value is not a byte array");
		auto& arr = std::get<ByteArray>(element.value);
		return std::string(this->_buffer.substr(arr.start, arr.len));
	}

	static UTFView ParseUTF(std::string_view buffer);
};

}


#endif