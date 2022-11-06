
#ifndef _ASSET_UPDATER_TASK_DISPATCHER_HPP_
#define _ASSET_UPDATER_TASK_DISPATCHER_HPP_

#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>


namespace AssetUpdater
{

class AssetDownloader;
class AssetUnpacker;
class AcbDecoder;

class TaskDispatcher
{
protected:

	enum STATUS 
	{
		DOWNLOAD = 0,
		UNPACK,
		DECODE,
		CLEANUP,
		FINISHED,
		FAILED
	};

	struct Task
	{
		std::string key;
		STATUS status = STATUS::DOWNLOAD;
		std::string message;	// Set when status = failed
	};
	
	size_t _total{};
	size_t _success{};
	size_t _failed{};
	const std::filesystem::path _PathPrefix;

	mutable std::mutex _mtx;
	mutable std::condition_variable _cv;

	friend class AssetDownloader;
	friend class AssetUnpacker;
	friend class AcbDecoder;

	std::unique_ptr<AssetDownloader> _downloader;
	std::unique_ptr<AssetUnpacker> _unpacker;
	std::unique_ptr<AcbDecoder> _decoder;

	void ReturnTask(Task task);

	STATUS _GetStatus(const std::string& key);
	void _DispatchTask(Task task);
	void _CleanUp(std::string key);

public:
	TaskDispatcher(
		size_t PoolSize, 
		std::string pymodule,
		std::string ApiUrl,
		std::filesystem::path UrlPrefix,
		std::filesystem::path PathPrefix,
		std::string cookie
	);

	TaskDispatcher(const TaskDispatcher&) = delete;
	TaskDispatcher& operator=(const TaskDispatcher&) = delete;
	TaskDispatcher(TaskDispatcher&&) = delete;
	TaskDispatcher& operator=(TaskDispatcher&&) = delete;

	~TaskDispatcher();


	void AddTask(std::string key);
	void WaitForFinish()
	{
		std::unique_lock<std::mutex> lk(this->_mtx);
		this->_cv.wait(lk, 
			[this]{ return this->_success + this->_failed >= this->_total; });
	}

	void PrintProgress() const;
};

}

#endif