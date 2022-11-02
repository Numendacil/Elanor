#ifndef _PIXIV_SINGLETON_HPP_
#define _PIXIV_SINGLETON_HPP_

#include <string>
#include <memory>

namespace Pixiv
{

class PixivClient;

std::shared_ptr<PixivClient> GetClient(std::string token, std::string ProxyHost = {}, int ProxyPort = -1);

}

#endif