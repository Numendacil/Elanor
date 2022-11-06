#include "AssetDownloader.hpp"

#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <httplib.h>

#include <PluginUtils/NetworkUtils.hpp>
#include <PluginUtils/Common.hpp>
#include "TaskDispatcher.hpp"

namespace filesystem = std::filesystem;
using std::string;

namespace AssetUpdater
{

AssetDownloader::AssetDownloader(
	std::size_t PoolSize, 
	std::string ApiUrl,
	std::filesystem::path UrlPrefix,
	std::filesystem::path PathPrefix,
	std::string cookie,
	TaskDispatcher* dispatcher
)
: _UrlPrefix(std::move(UrlPrefix)), _PathPrefix(std::move(PathPrefix)), 
  _ApiUrl(std::move(ApiUrl)), _cookie(std::move(cookie)),
  _dispatcher(dispatcher)
{
	this->_workers.reserve(PoolSize);
	for (std::size_t i = 0; i < PoolSize; i++)
	{
		_workers.emplace_back([this]() { this->_loop(); });
	}
}

AssetDownloader::~AssetDownloader()
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

void AssetDownloader::download(std::string key)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	this->_workloads.emplace(std::move(key));
	this->_cv.notify_one();
}

namespace
{

string GetTimestamp()
{
	using namespace std::chrono;
	auto tp = system_clock::now();
	std::time_t t = system_clock::to_time_t(tp + 1h);	// To JST

	constexpr size_t BUFFER_SIZE = 32;

	char buf[BUFFER_SIZE]; // NOLINT(*-avoid-c-arrays)
	std::strftime(buf, BUFFER_SIZE, "%Y%m%d%H%M%S", std::localtime(&t));
	return {buf};
}

}

void AssetDownloader::_loop()
{
	httplib::Client cli(this->_ApiUrl);
	cli.set_default_headers(httplib::Headers{
		{"Accept", "*/*"},
		{"Cookie", this->_cookie},
		{"Accept_Encoding", "gzip, deflate"},
		{"User-Agent", "ProductName/94 CFNetwork/1335.0.3 Darwin/21.6.0"},
		{"X-Unity-Version", "2020.3.32f1"},
		// {"Host", "production-" + this->_AssetbundleHostHash + "-assetbundle.sekai.colorfulpalette.org"}
	});
	cli.set_keep_alive(true);
	cli.set_decompress(true);
	cli.set_connection_timeout(10);
	cli.set_read_timeout(300);
	cli.set_write_timeout(10);

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
				return;
			key = std::move(this->_workloads.front());
			this->_workloads.pop();
		}
	
	try
	{
		const auto unpack_folder = this->_PathPrefix / key;
		const auto final_folder = this->_PathPrefix / (key + "_rip");
		filesystem::remove_all(unpack_folder);
		filesystem::remove_all(final_folder);
		
		auto filepath = this->_PathPrefix / (key + ".pack");
		string url = this->_UrlPrefix / key;
		filesystem::create_directories(filepath.parent_path());

		Utils::RunWithRetry([&]
		{
			std::ofstream ofile(filepath, std::ios::binary | std::ios::out);

			bool first_block = true;
			string block_head;
			block_head.reserve(128 + 4);

			auto result = cli.Get(url + "?t=" + GetTimestamp(), 
			[](const httplib::Response &response) -> bool
			{
				if (response.status < 200 || response.status > 299)	// NOLINT(*-avoid-magic-numbers)
					return false;
				return true;
			},
			[&](const char* data, size_t n) -> bool
			{
				if (first_block)
				{
					block_head.append(data, n);
					if (block_head.size() < 128 + 4)
						return true;
					
					std::string_view view = block_head;
					if (view.substr(0, 7) != "UnityFS")
					{
						view = view.substr(4);
						assert(view.substr(5, 2) == "FS");

						for (uint8_t i = 0; i < 128; i += 8)
						{
							ofile << static_cast<char>(~view[i]);
							ofile <<  static_cast<char>(~view[i + 1]);
							ofile <<  static_cast<char>(~view[i + 2]);
							ofile <<  static_cast<char>(~view[i + 3]);
							ofile <<  static_cast<char>(~view[i + 4]);
							ofile << view[i + 5];
							ofile << view[i + 6];
							ofile << view[i + 7];
						}
						view = view.substr(128); 
					}
					ofile << view;
					block_head.clear();
					first_block = false;
				}
				else
					ofile.write(data, static_cast<long>(n));
				return true;
			}
			);

			if (!Utils::VerifyResponse(result))
					throw Utils::NetworkException(result);
		});
	}
	catch(const std::exception& e)
	{
		this->_dispatcher->ReturnTask({std::move(key), TaskDispatcher::FAILED, e.what()});
		continue;
	}

		this->_dispatcher->ReturnTask({std::move(key), TaskDispatcher::UNPACK, {}});
	
	}

}

} // namespace AssetUpdater