#ifndef _PCM2FLAC_ENCODE_HPP_
#define _PCM2FLAC_ENCODE_HPP_

#include <vector>
#include <cstdint>
#include <filesystem>

namespace pcm2mp3
{

void EncodePCM(
	const std::vector<float>& pcm_data, 
	uint8_t ChannelCount, uint32_t SamplingRate,
	const std::filesystem::path& output
);


}


#endif