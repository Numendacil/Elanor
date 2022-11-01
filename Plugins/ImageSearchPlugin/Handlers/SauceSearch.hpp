#ifndef _IMAGESEARCH_HANDLERS_SAUCE_HPP_
#define _IMAGESEARCH_HANDLERS_SAUCE_HPP_

#include <libmirai/models.hpp>
#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>

namespace SearchHandlers
{

void SearchSauce(std::string url, const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                 Utils::BotConfig& config);

}

#endif