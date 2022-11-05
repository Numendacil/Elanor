#ifndef _ASSET_UPDATER_ASSET_UNPACKER_HPP_
#define _ASSET_UPDATER_ASSET_UNPACKER_HPP_

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <vector>

namespace AssetUpdater
{

class TaskDispatcher;

class AssetUnpacker
{
protected:
	mutable std::mutex _mtx;
	mutable std::condition_variable _cv;

	std::vector<std::thread> _workers;
	std::queue<std::string> _workloads;

	const std::string _pymodule;
	const std::filesystem::path _path_prefix;

	bool _stop = false;

	void _loop();

	TaskDispatcher* _dispatcher;

public:
	AssetUnpacker(
		std::size_t pool_size, 
		std::string pymodule,
		std::filesystem::path path_prefix,
		TaskDispatcher* dispatcher
	);

	AssetUnpacker(const AssetUnpacker&) = delete;
	AssetUnpacker& operator=(const AssetUnpacker&) = delete;
	AssetUnpacker(AssetUnpacker&&) = delete;
	AssetUnpacker& operator=(AssetUnpacker&&) = delete;

	~AssetUnpacker();

	void unpack(std::string key);

};

}
#endif