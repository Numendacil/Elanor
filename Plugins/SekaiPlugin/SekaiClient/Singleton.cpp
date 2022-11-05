#include <memory>
#include <mutex>
#include <string>
#include <tuple>

#include "SekaiClient.hpp"
#include "Singleton.hpp"
#include <Core/Utils/Logger.hpp>

namespace Sekai
{

std::shared_ptr<SekaiClient> GetClient(
	std::filesystem::path AssetFolder,
	std::filesystem::path TemporaryFolder,
	std::filesystem::path UpdaterPath,
	std::string PythonModule,
	std::vector<std::string> filter,
	std::string AESKey, std::string AESIV,
	size_t PoolSize,
	std::string ProxyHost, int ProxyPort
)
{
	static std::mutex mtx;
	static std::tuple<
		std::filesystem::path,
		std::filesystem::path,
		std::filesystem::path,
		std::string,
		std::vector<std::string>,
		std::string, std::string,
		size_t,
		std::string, int
	> config;
	static std::shared_ptr<SekaiClient> client;

	std::lock_guard<std::mutex> lk(mtx);
	if (!client || config != std::tie(
		AssetFolder, 
		TemporaryFolder,
		UpdaterPath,
		PythonModule,
		filter,
		AESKey, AESIV, 
		PoolSize,
		ProxyHost, ProxyPort
	))
	{
		config = std::make_tuple(
			AssetFolder, 
			TemporaryFolder,
			UpdaterPath,
			PythonModule,
			filter,
			AESKey, AESIV, 
			PoolSize,
			ProxyHost, ProxyPort
		);
		client = std::make_shared<SekaiClient>(
			std::move(AssetFolder), 
			std::move(TemporaryFolder),
			std::move(UpdaterPath),
			std::move(PythonModule),
			std::move(filter),
			std::move(AESKey), std::move(AESIV), 
			PoolSize,
			std::move(ProxyHost), ProxyPort);
		LOG_DEBUG(Utils::GetLogger(), "New SekaiClient generated <SekaiClient>");
	}
	return client;
}

}
