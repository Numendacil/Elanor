#ifndef _PIXIV_SINGLETON_HPP_
#define _PIXIV_SINGLETON_HPP_

#include <string>
#include <memory>
#include <Core/Utils/Common.hpp>

namespace Pixiv
{

class PixivClient;

std::shared_ptr<PixivClient> GetClient(std::string token, std::string ProxyHost = {}, int ProxyPort = -1);

inline std::shared_ptr<PixivClient> GetClient(const Utils::BotConfig& config) 
{
	if (config.IsNull("/pixiv/proxy"))
		return GetClient(config.Get("/pixiv/token", ""));
	else
		return GetClient(
			config.Get("/pixiv/token", ""), 
			config.Get("/pixiv/proxy/host", config.Get("/proxy/host", "")), 
			config.Get("/pixiv/proxy/port", config.Get("/proxy/port", -1))
		);
}

}

#endif