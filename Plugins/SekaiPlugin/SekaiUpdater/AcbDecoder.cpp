#include "AcbDecoder.hpp"

#include <array>
#include <bit>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/wait.h>

#include <cricpp/hca.hpp>
#include <cricpp/acb.hpp>

#include "TaskDispatcher.hpp"

#include "utils.hpp"

#include "pcm2mp3/encode.hpp"

namespace filesystem = std::filesystem;
using std::string;

namespace AssetUpdater
{

AcbDecoder::AcbDecoder(
	std::size_t PoolSize, 
	std::filesystem::path PathPrefix,
	// uint64_t decrypt_key,
	TaskDispatcher* dispatcher
)
: _PathPrefix(std::move(PathPrefix)), /*_decrypt_key(decrypt_key),*/ _dispatcher(dispatcher)
{
	this->_workers.reserve(PoolSize);
	for (std::size_t i = 0; i < PoolSize; i++)
	{
		_workers.emplace_back([this]() { this->_loop(); });
	}
}

AcbDecoder::~AcbDecoder()
{
	{
		std::unique_lock<std::mutex> lk(this->_mtx);
		this->_stop = true;
	}
	this->_cv.notify_all();
	for (std::thread& th : this->_workers)
	{
		if (th.joinable()) 
			th.join();
	}
}

void AcbDecoder::decode(std::string key)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	this->_workloads.emplace(std::move(key));
	this->_cv.notify_one();
}

void AcbDecoder::_loop()
{
	while (true)
	{
		string key;
		{
			std::unique_lock<std::mutex> lk(this->_mtx);
			this->_cv.wait(lk, 
				[this] { 
					return this->_stop || !this->_workloads.empty(); 
				}	
			);
			if (this->_stop) 
				break;
			key = std::move(this->_workloads.front());
			this->_workloads.pop();
		}
	try
	{
		auto output_folder = this->_PathPrefix / key;
		for (const auto& extracted: filesystem::recursive_directory_iterator(output_folder))
		{
			if(!extracted.is_regular_file())
				continue;
			auto filename = extracted.path().filename().string();
			std::string_view view = filename;
			constexpr std::string_view suffix = ".acb.bytes";
			if (filename.size() < suffix.size() || view.substr(filename.size() - suffix.size(), suffix.size()) != suffix)
				continue;
			
			std::string output_file = filename.substr(0, filename.size() - suffix.size()) + ".mp3";

		try
		{
			std::string content;
			{
				std::ifstream ifile(extracted.path());

				constexpr size_t BUFFER_SIZE = 4096;
				char buffer[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)
				while(ifile.read(buffer, BUFFER_SIZE))
					content.append(buffer, BUFFER_SIZE);
				content.append(buffer, ifile.gcount());
			}
			auto acb = cricpp::ACBView::ParseACB(content);
			auto files = acb.GetHcaFiles();
			for (const auto& str : files)
			{
				auto hca = cricpp::HCAView::ParseHCA(str, 0/*this->_decrypt_key*/, acb.GetAwbKey());
				if (!hca.isParsed())
					throw std::runtime_error("Failed to parse hca");

				auto pcm = hca.DecodeToPCM();
				pcm2mp3::EncodePCM(pcm, hca.GetChannelCount(), hca.GetSamplingRate(), extracted.path().parent_path() / output_file);
			}
		}
		catch(const std::exception& e)
		{
			LOGGING("Failed to convert acb file: " + string(e.what()));
			filesystem::rename(extracted.path(), extracted.path().string() + ".unknown");
		}
			filesystem::remove(extracted.path());

		}
	}
	catch(const std::exception& e)
	{
		this->_dispatcher->ReturnTask({std::move(key), TaskDispatcher::FAILED, e.what()});
		continue;
	}

		this->_dispatcher->ReturnTask({std::move(key), TaskDispatcher::CLEANUP, {}});
	}

}

}