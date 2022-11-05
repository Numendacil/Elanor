
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <sys/wait.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavformat/avio.h>
#include <libavutil/samplefmt.h>
#include <libavutil/log.h>
}

#include "encode.hpp"

namespace pcm2mp3
{

namespace
{

av_always_inline std::string av_err2string(int errnum)
{
	char str[AV_ERROR_MAX_STRING_SIZE];	// NOLINT(*-avoid-c-arrays)
	return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

bool CheckSampleFormat(const AVCodec* codec, AVSampleFormat format)
{
	const auto* p = codec->sample_fmts;
	while (*p > 0)
	{
		if (*p == format) return true;
		p++;
	}
	return false;
}

uint64_t GetChannelLayout(const AVCodec* codec, uint8_t target)
{
	switch(target)
	{
	case 1:
		return AV_CH_LAYOUT_MONO;
	case 2:
		return AV_CH_LAYOUT_STEREO;
	default:
		break;
	}

	const auto* p = codec->channel_layouts;
	if (!p)
		throw std::runtime_error("No layout found with given channel: " + std::to_string(target));
	
	while (*p)
	{
		int nb_channels = av_get_channel_layout_nb_channels(*p);
		if (nb_channels == target) return *p;
		p++;
	}
	throw std::runtime_error("No layout found with given channel: " + std::to_string(target));
}

void ReadPacket(AVCodecContext* ctx, AVPacket* pkt, AVFormatContext* output_ctx)
{
	while (true)
	{
		int ret = avcodec_receive_packet(ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return;
		else if (ret < 0)
			throw std::runtime_error("Error encoding audio frame: " + av_err2string(ret));

		av_write_frame(output_ctx, pkt);
		av_packet_unref(pkt);
	}
}
void EncodeFrame(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, AVFormatContext* output_ctx)
{
	while (true)
	{
		int ret = avcodec_send_frame(ctx, frame);
		if (ret == AVERROR(EAGAIN))
			ReadPacket(ctx, pkt, output_ctx);
		else if (ret < 0)
			throw std::runtime_error("Error sending the frame to the encoder: " + av_err2string(ret));
		else
			break;
	}

	ReadPacket(ctx, pkt, output_ctx);
}

template <typename T>
struct ptr_guard
{
	T* ptr;
	std::function<void(T*)> deleter;

	ptr_guard(T* ptr = nullptr, std::function<void(T*)> deleter = std::default_delete<T>())
	: ptr(ptr), deleter(std::move(deleter)) {}

	ptr_guard(const ptr_guard&) = delete;
	ptr_guard& operator=(const ptr_guard&) = delete;
	ptr_guard(ptr_guard&& rhs) : ptr(rhs.ptr), deleter(std::move(rhs.deleter))
	{
		rhs.ptr = nullptr;
	}
	ptr_guard& operator=(ptr_guard&& rhs)
	{
		if (this != &rhs)
		{
			ptr = rhs.ptr;
			deleter = std::move(rhs.deleter);
			rhs.ptr = nullptr;
		}
		return *this;
	}

	~ptr_guard()
	{
		if (ptr) deleter(ptr);
	}

	T* operator->() const { return ptr; }
	operator bool () const { return ptr != nullptr; }
};

}

void EncodePCM(const std::vector<float>& pcm_data, uint8_t channels, uint32_t sample_rate,
               const std::filesystem::path& output)
{
	av_log_set_level(AV_LOG_QUIET);
	
	// codec setup
	const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
	if (!codec) 
		throw std::runtime_error("Failed to find codec");

	
	// codec context setup
	ptr_guard<AVCodecContext> codec_ctx(
		avcodec_alloc_context3(codec),
		[](AVCodecContext* ctx) { 
			avcodec_free_context(&ctx); 
		}
	);
	if (!codec_ctx) 
		throw std::runtime_error("Failed to allocate audio codec context");

	if (!CheckSampleFormat(codec, AV_SAMPLE_FMT_FLTP)) 
		throw std::runtime_error("Codec does not support fltp format");
	codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	codec_ctx->sample_rate = static_cast<int>(sample_rate);
	codec_ctx->channel_layout = GetChannelLayout(codec, channels);
	codec_ctx->channels = channels;
	codec_ctx->bit_rate = 192 * 1000;

	if (av_get_channel_layout_nb_channels(codec_ctx->channel_layout) != codec_ctx->channels)
		throw std::runtime_error("channel numbers dismatch");

	if (avcodec_open2(codec_ctx.ptr, codec, nullptr) < 0) 
		throw std::runtime_error("Failed to open codec");


	// output contex setup
	ptr_guard<AVFormatContext> output_ctx(
		nullptr,
		[](auto* ptr)
		{ 
			avio_close(ptr->pb);
			avformat_free_context(ptr);
		}
	);
	int ret = avformat_alloc_output_context2(&output_ctx.ptr, nullptr, nullptr, output.c_str());
	if (ret != 0)
		throw std::runtime_error("Failed to alloc output context: " + av_err2string(ret));
	AVStream* ostream = avformat_new_stream(output_ctx.ptr, codec);
	if (!ostream)
		throw std::runtime_error("Failed to add output stream");
	ostream->id = 0;
	avcodec_parameters_from_context(ostream->codecpar, codec_ctx.ptr);

	ret = avio_open(&output_ctx->pb, output_ctx->url, AVIO_FLAG_WRITE);
	if (ret != 0) 
		throw std::runtime_error("Failed to open output io: " + av_err2string(ret));


	// write header
	ret = avformat_write_header(output_ctx.ptr, nullptr);
	if (ret != 0) 
		throw std::runtime_error("Failed to write header: " + av_err2string(ret));


	// init frame and packet
	ptr_guard<AVPacket> pkt(
		av_packet_alloc(), 
		[](auto* pkt) { av_packet_free(&pkt); }
	);
	if (!pkt) 
		throw std::runtime_error("Failed to allocate packet");

	ptr_guard<AVFrame> frame(
		av_frame_alloc(), 
		[](auto* frame) { av_frame_free(&frame); }
	);
	if (!frame) 
		throw std::runtime_error("Failed to allocate audio frame");

	frame->nb_samples = codec_ctx->frame_size;
	frame->format = codec_ctx->sample_fmt;
	frame->channel_layout = codec_ctx->channel_layout;
	frame->pts = 0;
	
	ret = av_frame_get_buffer(frame.ptr, 0);
	if (ret < 0) 
		throw std::runtime_error("Failed to allocate audio buffers: " + av_err2string(ret));


	float* samples[8];	// NOLINT(*-avoid-c-arrays)
	for (int i = 0; i < 8; i++)
	 	samples[i] = reinterpret_cast<float*>(frame->data[i]);	// NOLINT(*-constant-array-index)
	auto* data = pcm_data.data();
	int j = 0;


	for (size_t i = 0; i < pcm_data.size() / channels; i++)
	{
		for (int k = 0; k < channels; k++)
		{
			// NOLINTNEXTLINE(*-constant-array-index)
			*samples[k] = *data;

			samples[k]++;	// NOLINT(*-constant-array-index)
			data++;
		}

		j++;
		if (j >= codec_ctx->frame_size)
		{
			j = 0;
			// std::cout << i << ' ' << codec_ctx->frame_size << std::endl;
			EncodeFrame(codec_ctx.ptr, frame.ptr, pkt.ptr, output_ctx.ptr);
			ret = av_frame_make_writable(frame.ptr);
			if (ret < 0) 
				throw std::runtime_error("Frame not writable: " + av_err2string(ret));
			for (int i = 0; i < 8; i++)
	 			samples[i] = reinterpret_cast<float*>(frame->data[i]);	// NOLINT(*-constant-array-index)
			frame->pts = static_cast<int64_t>(i + 1);
		}
	}
	if (j)
	{
		frame->nb_samples = j;
		EncodeFrame(codec_ctx.ptr, frame.ptr, pkt.ptr, output_ctx.ptr);
	}
	EncodeFrame(codec_ctx.ptr, nullptr, pkt.ptr, output_ctx.ptr);
	av_write_trailer(output_ctx.ptr);
	avformat_flush(output_ctx.ptr);
}

}