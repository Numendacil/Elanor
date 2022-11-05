#ifndef _CRICPP_HCA_HPP_
#define _CRICPP_HCA_HPP_

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>
#include <optional>
#include <cassert>
#include <array>
#include <vector>

namespace cricpp
{

class HCAView
{
protected:
	struct Header
	{
		uint32_t magic;		// LE
		uint16_t version;	// BE
		uint16_t DataOffset;	// BE
	};

	static_assert(std::is_standard_layout_v<Header> && std::is_trivial_v<Header>, "Not POD type");

	struct Format
	{
		uint32_t format;		// LE
		uint32_t ChannelCountSamplingRate;		// BE
		uint32_t BlockCount;		// BE
		uint16_t MuteHeader;		// BE
		uint16_t MuteFooter;		// BE

		uint8_t GetChannelCount() const
		{
			return ChannelCountSamplingRate >> 24;
		}
		uint32_t GetSamplingRate() const
		{
			return ChannelCountSamplingRate & 0xFFFFFF;
		}
	};

	static_assert(std::is_standard_layout_v<Format> && std::is_trivial_v<Format>, "Not POD type");


	struct CompressInfo
	{
		uint32_t label;		// LE
		uint16_t BlockSize;	// BE
		uint8_t r01;
		uint8_t r02;
		uint8_t r03;
		uint8_t r04;
		uint8_t r05;
		uint8_t r06;
		uint8_t r07;
		uint8_t r08;
		uint8_t reserve1;
		uint8_t reserve2;
	};

	static_assert(std::is_standard_layout_v<CompressInfo> && std::is_trivial_v<CompressInfo>, "Not POD type");

	struct DecodeInfo
	{
		uint32_t label;		// LE
		uint16_t BlockSize;	// BE
		uint8_t r01;
		uint8_t r02;
		uint8_t count1;
		uint8_t count2;
		uint8_t r0304;
		uint8_t EnableCount2;

		uint8_t GetR03() { return r0304 >> 4 & 0xF; }
		uint8_t GetR04() { return r0304 & 0xF; }
	};

	static_assert(std::is_standard_layout_v<DecodeInfo> && std::is_trivial_v<DecodeInfo>, "Not POD type");

	struct VBRInfo
	{
		uint32_t label;		// LE
		uint16_t r01;		// BE
		uint16_t r02;		// BE
	};

	static_assert(std::is_standard_layout_v<VBRInfo> && std::is_trivial_v<VBRInfo>, "Not POD type");

	struct ATHInfo
	{
		uint32_t label;		// LE
		uint16_t type;		// BE
	};

	static_assert(std::is_standard_layout_v<ATHInfo> && std::is_trivial_v<ATHInfo>, "Not POD type");

	struct LoopInfo
	{
		uint32_t label;		// LE
		uint32_t start;		// BE
		uint32_t end;		// BE
		uint16_t count;		// BE
		uint16_t r01;		// BE
	};

	static_assert(std::is_standard_layout_v<LoopInfo> && std::is_trivial_v<LoopInfo>, "Not POD type");

	struct CipherInfo
	{
		uint32_t label;		// LE
		uint16_t type;		// BE
	};

	static_assert(std::is_standard_layout_v<CipherInfo> && std::is_trivial_v<CipherInfo>, "Not POD type");

	struct RVAInfo
	{
		uint32_t label;		// LE
		float volume;		// BE
	};

	static_assert(std::is_standard_layout_v<RVAInfo> && std::is_trivial_v<RVAInfo>, "Not POD type");

	struct CommentInfo
	{
		uint32_t label;		// LE
		uint8_t len;
	};

	static_assert(std::is_standard_layout_v<CommentInfo> && std::is_trivial_v<CommentInfo>, "Not POD type");

	struct PadInfo
	{
		uint32_t label;		// LE
	};

	static_assert(std::is_standard_layout_v<PadInfo> && std::is_trivial_v<PadInfo>, "Not POD type");

	class BlockReader
	{
	protected:
		std::string_view _buffer;
		size_t _pos{};		// in bits
		static constexpr std::array<uint32_t, 8> _mask = {
			0xFFFFFF, 0x7FFFFF, 0x3FFFFF, 0x1FFFFF,
			0x0FFFFF, 0x07FFFF, 0x03FFFF, 0x01FFFF
		};

		size_t _size() const { return _buffer.size() * 8 - 16; }
	
	public:
		BlockReader(std::string_view buffer)
		: _buffer(buffer) {}

		uint32_t PeekBit(size_t bitsize) const
		{
			if (bitsize + this->_pos > this->_size())
				return 0;

			uint32_t v{};
			size_t bytepos = this->_pos >> 3;	// div 8
			uint8_t bitpos = this->_pos & 0x7;	// mod 8
			if (24 - bitsize - bitpos < 0)
				throw std::runtime_error("overflow");
			const auto* p = 
				reinterpret_cast<const unsigned char*>(this->_buffer.data()) + bytepos;
			v = p[0];
			v = (v << 8) | p[1];
			v = (v << 8) | p[2];
			v = v & this->_mask[bitpos];	// NOLINT(*-constant-array-index)
			v = v >> (24 - bitsize - bitpos);

			return v;
		}

		uint32_t ReadBit(size_t bitsize)
		{
			auto v = this->PeekBit(bitsize);
			this->_pos += bitsize;
			return v;
		}

		void SkipBit(int64_t bitsize)
		{
			this->_pos += bitsize;
		}

	};

	struct Channel
	{
		std::array<float, 0x80> block{};
		std::array<float, 0x80> base{};
		std::array<uint8_t, 0x80> value{};
		std::array<uint8_t, 0x80> scale{};
		std::array<uint8_t, 8> value2{};
		uint8_t type{};
		uint8_t value3_offset{};
		uint8_t count{};
		std::array<float, 0x80> wav1{};
		std::array<float, 0x80> wav2{};
		std::array<float, 0x80> wav3{};
		std::array<std::array<float, 0x80>, 8> wav{};
	};


	std::optional<VBRInfo> _vbr;
	std::optional<ATHInfo> _ath;
	std::optional<LoopInfo> _loop;
	std::optional<CipherInfo> _cipher;
	std::optional<RVAInfo> _rva;
	std::optional<CommentInfo> _comment;
	std::optional<std::string> _CommentStr;
	std::optional<PadInfo> _pad;

	std::array<uint8_t, 128> _AthTable{};
	std::array<uint8_t, 256> _CiphTable{};

	std::vector<Channel> _channels;

	Header _header{};
	Format _format{};
	uint32_t _compdec{};
	uint16_t _BlockSize{};
	uint8_t _comp_r01{};
	uint8_t _comp_r02{};
	uint8_t _comp_r03{};
	uint8_t _comp_r04{};
	uint8_t _comp_r05{};
	uint8_t _comp_r06{};
	uint8_t _comp_r07{};
	uint8_t _comp_r08{};
	uint8_t _comp_r09{};

	std::string_view _buffer;

	bool _SetAthTable(uint16_t type, uint32_t key);
	bool _SetCiphTable(uint16_t type, uint32_t key1, uint32_t key2);

	void _Decode1(Channel& channel, BlockReader& reader, uint32_t a, uint32_t b);
	void _Decode2(Channel& channel, BlockReader& reader);
	void _Decode3(Channel& channel, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
	void _Decode4(Channel& channel, Channel& NextChannel, size_t index, uint32_t a, uint32_t b, uint32_t c);
	void _Decode5(Channel& channel, size_t index);

public:
	bool isParsed() const { return !this->_buffer.empty(); }

	std::vector<float> DecodeToPCM();

	uint8_t GetChannelCount() const
	{
		return this->_format.GetChannelCount();
	}
	uint32_t GetSamplingRate() const
	{
		return this->_format.GetSamplingRate();
	}

	static HCAView ParseHCA(std::string_view buffer, uint64_t key, uint16_t AwbKey = 0);
};

}

#endif