#ifndef _PIXIV_FUNCTIONS_PIXIVID_HPP_
#define _PIXIV_FUNCTIONS_PIXIVID_HPP_

#include <string>
#include <vector>

#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>

namespace Pixiv::PixivId
{

constexpr std::string_view COMMAND_NAME= "id";

constexpr std::string_view HELP_TEXT = 
"\n#pixiv id [pid] (page/all)";

void GetIllustById(const std::vector<std::string>& tokens, const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                   Utils::BotConfig& config);

}

#endif