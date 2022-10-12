#include <memory>
#include <mutex>
#include <string>
#include <tuple>

#include "PixivClient.hpp"
#include "Singleton.hpp"
#include <Core/Utils/Logger.hpp>

namespace Pixiv
{

std::shared_ptr<PixivClient> GetClient(std::string token, std::string proxy_host, int proxy_port)
{
	static std::mutex mtx;
	static std::tuple<std::string, std::string, int> config;
	static std::shared_ptr<PixivClient> client;

	std::lock_guard<std::mutex> lk(mtx);
	if (!client || config != std::tie(token, proxy_host, proxy_port))
	{
		config = std::make_tuple(token, proxy_host, proxy_port);
		client = std::make_shared<PixivClient>(std::move(token), std::move(proxy_host), proxy_port);
		LOG_DEBUG(Utils::GetLogger(), "New PixivClient generated <PixivClient>");
	}
	return client;
}

}
