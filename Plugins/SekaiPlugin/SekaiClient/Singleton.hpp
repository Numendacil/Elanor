#ifndef _SEKAI_SINGLETON_HPP_
#define _SEKAI_SINGLETON_HPP_

#include <filesystem>
#include <string>
#include <memory>
#include <vector>
#include <Core/Utils/Common.hpp>

namespace Sekai
{

class SekaiClient;

std::shared_ptr<SekaiClient> GetClient(
	std::filesystem::path AssetFolder,
	std::filesystem::path TemporaryFolder,
	std::filesystem::path UpdaterPath,
	std::string PythonModule,
	std::vector<std::string> filter,
	std::string AESKey, std::string AESIV,
	std::string ApiUrl,
	size_t PoolSize = 4,
	std::string ProxyHost = {}, int ProxyPort = -1
);

inline std::shared_ptr<SekaiClient> GetClient(const Utils::BotConfig& config)
{
	if (config.IsNull("/sekai/proxy"))
		return GetClient(
			config.Get("/path/MediaFiles", "MediaFiles") / std::filesystem::path("pjsk"), 
			config.Get("/path/MediaFiles", "MediaFiles") / std::filesystem::path("tmp"), 
			config.Get("/sekai/UpdaterPath", "SekaiUpdater"), 
			config.Get("/path/pymodules", "pymodules") + ".sekai-extract", 
			config.Get("/sekai/filter", std::vector<std::string>{}), 
			config.Get("/sekai/AESKey", ""), 
			config.Get("/sekai/AESIV", ""),
			config.Get("/sekai/api", ""),
			config.Get("/sekai/PoolSize", 4)
		);
	else
		return GetClient(
			config.Get("/path/MediaFiles", "MediaFiles") / std::filesystem::path("pjsk"), 
			config.Get("/path/MediaFiles", "MediaFiles") / std::filesystem::path("tmp"), 
			config.Get("/sekai/UpdaterPath", "SekaiUpdater"), 
			config.Get("/path/pymodules", "pymodules") + ".sekai-extract", 
			config.Get("/sekai/filter", std::vector<std::string>{}), 
			config.Get("/sekai/AESKey", ""), 
			config.Get("/sekai/AESIV", ""),
			config.Get("/sekai/api", ""),
			config.Get("/sekai/PoolSize", 4),
			config.Get("/sekai/proxy/host", config.Get("/sekai/host", "")), 
			config.Get("/sekai/proxy/port", config.Get("/sekai/port", -1))
		);
}

}

#endif