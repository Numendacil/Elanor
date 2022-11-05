#ifndef _SEKAI_CLIENT_HPP_
#define _SEKAI_CLIENT_HPP_


#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <shared_mutex>

#include <nlohmann/json_fwd.hpp>

namespace Sekai
{

class SekaiNetworkClient;

class SekaiClient
{
protected:
	std::unique_ptr<SekaiNetworkClient> _cli;

	const std::filesystem::path _AssetFolder;
	const std::filesystem::path _TmpFolder;
	const std::filesystem::path _UpdaterPath;

	// const uint64_t _HCAKey;
	const size_t _PoolSize;
	const std::string _PyModule;
	const std::vector<std::string> _AssetFilter;

	mutable std::shared_mutex _mtx;

	void _UpdateMasterDB() const;
	void _UpdateAssets() const;

public:
	explicit SekaiClient(
		std::filesystem::path AssetFolder,
		std::filesystem::path TemporaryFolder,
		std::filesystem::path UpdaterPath,
		std::string PythonModule,
		std::vector<std::string> filter,
		std::string AESKey, std::string AESIV, 
		// uint64_t HCAKey,
		size_t PoolSize = 4,
		std::string ProxyHost = {}, int ProxyPort = -1
	);

	SekaiClient(const SekaiClient&) = delete;
	SekaiClient& operator=(const SekaiClient&) = delete;
	SekaiClient(SekaiClient&&) = delete;
	SekaiClient& operator=(SekaiClient&&) = delete;

	~SekaiClient();

	/* Sekai APIs */
	
	// TBD

	/* File related operations */

	nlohmann::json GetMetaInfo(const std::string& category);

	// Do not use the returned path to read from filesystem directly,
	// call GetFile() and GetFileContents() instead to avoid concurrency problems
	std::vector<std::filesystem::path> GetAssetContents(const std::string& key);

	// This method copies the contents to a temporary file
	std::filesystem::path GetFile(const std::string& key, const std::filesystem::path& file);

	std::string GetFileContents(const std::string& key, const std::filesystem::path& file);

	std::vector<std::string> UpdateContents() const;

};

}

#endif