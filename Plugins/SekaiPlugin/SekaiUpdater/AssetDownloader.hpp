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
	const std::filesystem::path _UrlPrefix;
	const std::filesystem::path _PathPrefix;
	const std::string _cookie;
	const std::string _ApiUrl;

	void _loop();

	TaskDispatcher* _dispatcher;

public:
	AssetDownloader(
		std::size_t PoolSize, 
		std::string ApiUrl,
		std::filesystem::path UrlPrefix,
		std::filesystem::path PathPrefix,
		std::string cookie,
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