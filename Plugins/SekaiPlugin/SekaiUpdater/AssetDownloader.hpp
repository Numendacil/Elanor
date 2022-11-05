#ifndef _ASSET_UPDATER_ASSET_DOWNLOADER_HPP_
#define _ASSET_UPDATER_ASSET_DOWNLOADER_HPP_

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <vector>

namespace AssetUpdater
{

class TaskDispatcher;

class AssetDownloader
{
protected:
	mutable std::mutex _mtx;
	mutable std::condition_variable _cv;

	std::vector<std::thread> _workers;
	std::queue<std::string> _workloads;

	bool _stop = false;
	const std::filesystem::path _url_prefix;
	const std::filesystem::path _path_prefix;
	const std::string _cookie;
	const std::string _AssetbundleHostHash;

	void _loop();

	TaskDispatcher* _dispatcher;

public:
	AssetDownloader(
		std::size_t pool_size, 
		std::filesystem::path url_prefix,
		std::filesystem::path path_prefix,
		std::string cookie,
		std::string AssetbundleHostHash,
		TaskDispatcher* dispatcher
	);

	AssetDownloader(const AssetDownloader&) = delete;
	AssetDownloader& operator=(const AssetDownloader&) = delete;
	AssetDownloader(AssetDownloader&&) = delete;
	AssetDownloader& operator=(AssetDownloader&&) = delete;

	~AssetDownloader();

	void download(std::string key);
};

}
#endif