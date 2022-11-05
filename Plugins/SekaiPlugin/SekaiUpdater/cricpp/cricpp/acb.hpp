#ifndef _CRICPP_ACB_HPP_
#define _CRICPP_ACB_HPP_

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "utf.hpp"
#include "afs2.hpp"

namespace cricpp
{

class ACBView
{
protected:
	std::string_view _buffer;

	UTFView _utf;
	ASF2View _afs2;

	struct Waveform
	{
		int8_t EncodeType{};
		int16_t ExtensionData{};
		int8_t LoopFlag{};
		int16_t MemoryAwbId{};
		int8_t NumChannels{};
		int32_t NumSamples{};
		int16_t SamplingRate{};
		int16_t StreamAwbId{};
		int16_t StreamAwbPortNo{};
		int8_t Streaming{};
	};

	std::vector<Waveform> _WaveformTable;
	std::string_view _StreamAwbHash;

	std::string_view _AwsFile;
	std::vector<std::string_view> _MemoryHcas;

public:
	bool isParsed() const 
	{ 
		return !this->_buffer.empty() 
			&& this->_utf.isParsed()
			&& this->_afs2.isParsed(); 
	}

	std::vector<std::string_view> GetHcaFiles() const
	{
		return _MemoryHcas;
	}

	uint16_t GetAwbKey() const
	{
		return _afs2.GetKey();
	}


	static ACBView ParseACB(std::string_view buffer);
};

}

#endif