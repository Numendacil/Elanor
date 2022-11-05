#ifndef _CRICPP_UTILS_HPP_
#define _CRICPP_UTILS_HPP_

#include <cstdint>
#include <bit>
#include <cstring>

namespace cricpp
{

inline void UInt16BE(uint16_t& src)
{
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
			"Mixed endian not supported");
	if constexpr (std::endian::native == std::endian::big)
	{
		return;
	}
	else
	{
		src = __builtin_bswap16(src);
	}
}

inline void UInt16LE(uint16_t& src)
{
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
			"Mixed endian not supported");
	if constexpr (std::endian::native == std::endian::big)
	{
		src = __builtin_bswap16(src);
	}
	else
	{
		return;
	}
}


inline void UInt32BE(uint32_t& src)
{
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
			"Mixed endian not supported");
	if constexpr (std::endian::native == std::endian::big)
	{
		return;
	}
	else
	{
		src = __builtin_bswap32(src);
	}
}

inline void UInt32LE(uint32_t& src)
{
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
			"Mixed endian not supported");
	if constexpr (std::endian::native == std::endian::big)
	{
		src = __builtin_bswap32(src);
	}
	else
	{
		return;
	}
}

inline void UInt64BE(uint64_t& src)
{
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
			"Mixed endian not supported");
	if constexpr (std::endian::native == std::endian::big)
	{
		return;
	}
	else
	{
		src = __builtin_bswap64(src);
	}
}

inline void UInt64LE(uint64_t& src)
{
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little,
			"Mixed endian not supported");
	if constexpr (std::endian::native == std::endian::big)
	{
		src = __builtin_bswap64(src);
	}
	else
	{
		return;
	}
}

inline void FloatBE(float& src)
{
	static_assert(sizeof(float) == 4, "float type is not 4 bytes");
	auto tmp = std::bit_cast<uint32_t>(src);
	UInt32BE(tmp);
	src = std::bit_cast<float>(tmp);
}

inline void FloatLE(float& src)
{
	static_assert(sizeof(float) == 4, "float type is not 4 bytes");
	auto tmp = std::bit_cast<uint32_t>(src);
	UInt32LE(tmp);
	src = std::bit_cast<float>(tmp);
}

inline void DoubleBE(double& src)
{
	static_assert(sizeof(double) == 8, "double type is not 8 bytes");
	auto tmp = std::bit_cast<uint64_t>(src);
	UInt64BE(tmp);
	src = std::bit_cast<double>(tmp);
}

inline void DoubleLE(double& src)
{
	static_assert(sizeof(double) == 8, "double type is not 8 bytes");
	auto tmp = std::bit_cast<uint64_t>(src);
	UInt64LE(tmp);
	src = std::bit_cast<double>(tmp);
}

inline uint16_t ReadUInt16BE(const void* src)
{
	uint16_t result{};
	memcpy(&result, src, sizeof(uint16_t));
	UInt16BE(result);
	return result;
}

inline uint16_t ReadUInt16LE(const void* src)
{
	uint16_t result{};
	memcpy(&result, src, sizeof(uint16_t));
	UInt16LE(result);
	return result;
}

inline uint32_t ReadUInt32BE(const void* src)
{
	uint32_t result{};
	memcpy(&result, src, sizeof(uint32_t));
	UInt32BE(result);
	return result;
}

inline uint32_t ReadUInt32LE(const void* src)
{
	uint32_t result{};
	memcpy(&result, src, sizeof(uint32_t));
	UInt32LE(result);
	return result;
}

inline uint64_t ReadUInt64BE(const void* src)
{
	uint64_t result{};
	memcpy(&result, src, sizeof(uint64_t));
	UInt64BE(result);
	return result;
}

inline uint64_t ReadUInt64LE(const void* src)
{
	uint64_t result{};
	memcpy(&result, src, sizeof(uint64_t));
	UInt64LE(result);
	return result;
}

inline float ReadFloatBE(const void* src)
{
	static_assert(sizeof(float) == 4, "float type is not 4 bytes");
	return std::bit_cast<float>(ReadUInt32BE(src));
}

inline float ReadFloatLE(const void* src)
{
	static_assert(sizeof(float) == 4, "float type is not 4 bytes");
	return std::bit_cast<float>(ReadUInt32LE(src));
}

inline double ReadDoubleBE(const void* src)
{
	static_assert(sizeof(double) == 8, "double type is not 8 bytes");
	return std::bit_cast<double>(ReadUInt64BE(src));
}

inline double ReadDoubleLE(const void* src)
{
	static_assert(sizeof(double) == 8, "double type is not 8 bytes");
	return std::bit_cast<double>(ReadUInt64LE(src));
}

}

#endif