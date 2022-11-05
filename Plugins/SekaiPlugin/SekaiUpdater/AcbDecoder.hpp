#ifndef _ASSET_UPDATER_ACB_DECODER_HPP_
#define _ASSET_UPDATER_ACB_DECODER_HPP_

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <vector>
#include <iostream>

namespace AssetUpdater
{

class TaskDispatcher;

class AcbDecoder
{
	protected:
	mutable std::mutex _mtx;
	mutable std::condition_variable _cv;

	std::vector<std::thread> _workers;
	std::queue<std::string> _workloads;

	bool _stop = false;
	const std::filesystem::path _path_prefix;
	// const uint64_t _decrypt_key{};

	void _loop();

	TaskDispatcher* _dispatcher;

public:
	AcbDecoder(
		std::size_t pool_size, 
		std::filesystem::path path_prefix,
		// uint64_t decrypt_key,
		TaskDispatcher* dispatcher
	);

	AcbDecoder(const AcbDecoder&) = delete;
	AcbDecoder& operator=(const AcbDecoder&) = delete;
	AcbDecoder(AcbDecoder&&) = delete;
	AcbDecoder& operator=(AcbDecoder&&) = delete;

	~AcbDecoder();

	void decode(std::string key);
};

}

#endif