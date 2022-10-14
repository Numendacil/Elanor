#include "PixivCommand.hpp"

#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/Client.hpp>
#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

#include <Functions/PixivId.hpp>

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool PixivCommand::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                     Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 2) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#pixiv" && command != "#p站") return false;


	LOG_INFO(Utils::GetLogger(), "Calling Pixiv <Pixiv>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <Pixiv>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}

	command = Utils::toLower(tokens[1]);
	if (command == "help" || command == "h" || command == "帮助")
	{
		LOG_INFO(Utils::GetLogger(), "帮助文档 <Pixiv>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, 
					Mirai::MessageChain()
					.Plain("usage:")
					.Plain(Pixiv::PixivId::HELP_TEXT.data())
					);
	}
	else if (command == Pixiv::PixivId::COMMAND_NAME)
	{
		Pixiv::PixivId::GetIllustById(tokens, gm, group, client, config);
	}
	else
	{	
		LOG_INFO(Utils::GetLogger(), "未知命令 <Pixiv>: " + command + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(command + "是什么指令捏，不知道捏"));
	}

	return true;
}

}