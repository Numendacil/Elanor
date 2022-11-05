#ifndef _SEKAI_SINGLETON_HPP_
#define _SEKAI_SINGLETON_HPP_

#include <filesystem>
#include <string>
#include <memory>
#include <vector>

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
	size_t PoolSize = 4,
	std::string ProxyHost = {}, int ProxyPort = -1
);

}

#endif