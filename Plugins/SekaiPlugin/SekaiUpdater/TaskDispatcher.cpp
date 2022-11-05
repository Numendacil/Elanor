#include "TaskDispatcher.hpp"

#include <cstddef>
#include <exception>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <filesystem>

#include <openssl/evp.h>

#include "AcbDecoder.hpp"
#include "AssetDownloader.hpp"
#include "AssetUnpacker.hpp"

#include "utils.hpp"

namespace filesystem = std::filesystem;
using std::string;

namespace AssetUpdater
{

TaskDispatcher::TaskDispatcher(
	size_t PoolSize, 
	string pymodule,
	filesystem::path UrlPrefix,
	filesystem::path PathPrefix,
	string cookie,
	string AssetbundleHostHash
): _PathPrefix(std::move(PathPrefix))
{
	this->_downloader = std::make_unique<AssetDownloader>(
		PoolSize, std::move(UrlPrefix), this->_PathPrefix, std::move(cookie), std::move(AssetbundleHostHash), this
	);
	this->_unpacker = std::make_unique<AssetUnpacker>(
		PoolSize, std::move(pymodule), this->_PathPrefix, this
	);
	this->_decoder = std::make_unique<AcbDecoder>(
		PoolSize, this->_PathPrefix, /*decrypt_key,*/ this
	);
}

TaskDispatcher::~TaskDispatcher() = default;

TaskDispatcher::STATUS TaskDispatcher::_GetStatus(const string& key)
{
	const auto dl_file = this->_PathPrefix / (key + ".pack");
	const auto unpack_folder = this->_PathPrefix / key;
	const auto final_folder = this->_PathPrefix / (key + "_rip");

	bool has_file = filesystem::exists(dl_file) && filesystem::is_regular_file(dl_file);
	bool has_unpack_folder = filesystem::exists(unpack_folder) && filesystem::is_directory(unpack_folder);
	bool has_final_folder = filesystem::exists(final_folder) && filesystem::is_directory(final_folder);

	// if (has_final_folder && !has_file && !has_unpack_folder)
	// 	return STATUS::FINISHED;

	if (has_final_folder && has_file)
		return STATUS::CLEANUP;

	if (!has_final_folder && has_unpack_folder && has_file)
		return STATUS::UNPACK;

	return STATUS::DOWNLOAD;
}

void TaskDispatcher::_CleanUp(string key)
{
	try
	{
		const auto dl_file = this->_PathPrefix / (key + ".pack");
		const auto unpack_folder = this->_PathPrefix / key;
		const auto final_folder = this->_PathPrefix / (key + "_rip");
		if (!filesystem::exists(final_folder))
			filesystem::rename(unpack_folder, final_folder);
			
		else if (!filesystem::is_directory(final_folder))
		{
			filesystem::remove(final_folder);
			filesystem::rename(unpack_folder, final_folder);
		}
		filesystem::remove_all(unpack_folder);
		filesystem::remove_all(dl_file);
	}
	catch(const std::exception& e)
	{
		this->ReturnTask({std::move(key), STATUS::FAILED, e.what()});
		return;
	}
	this->ReturnTask({std::move(key), STATUS::FINISHED, ""});
}

void TaskDispatcher::_DispatchTask(Task task)
{
	switch(task.status)
	{
	case DOWNLOAD:
		this->_downloader->download(std::move(task.key));
		break;
	case UNPACK:
		this->_unpacker->unpack(std::move(task.key));
		break;
	case DECODE:
		this->_decoder->decode(std::move(task.key));
		break;
	case CLEANUP:
		this->_CleanUp(std::move(task.key));
		break;
	case FINISHED:
		break;
	case FAILED:
		break;
	}
}

void TaskDispatcher::AddTask(string key)
{
	STATUS status = _GetStatus(key);

	// if (status == STATUS::FINISHED)
	// 	return false;
	this->_DispatchTask({std::move(key), status, ""});

	std::lock_guard<std::mutex> lk(this->_mtx);
	this->_total++;
	// return true;
}

void TaskDispatcher::ReturnTask(Task task)
{
	if (task.status == STATUS::FINISHED)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_success++;
		if (this->_failed + this->_success >= this->_total)
			this->_cv.notify_all();
		return;
	}

	if (task.status == STATUS::FAILED)
	{
		LOGGING("Task failed: " + task.message);
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_failed++;
		if (this->_failed + this->_success >= this->_total)
			this->_cv.notify_all();
		return;
	}

	this->_DispatchTask(std::move(task));
}

void TaskDispatcher::PrintProgress() const
{
	size_t total{}, success{}, failed{};
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		total = this->_total;
		success = this->_success;
		failed = this->_failed;
	}
	LOGGING("Current progress: success " 
		+ std::to_string(success)  + "/" + std::to_string(total)
		+ ", failed "
		+ std::to_string(failed)  + "/" + std::to_string(total));
}

}