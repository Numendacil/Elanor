#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/mirai.hpp>
#include <libmirai/Client.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

#include "Recall.hpp"

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool Recall::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                       Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') 
		return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 1) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#recall" && command != "#撤回") return false;


	LOG_INFO(Utils::GetLogger(), "Calling Recall <Recall>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <Recall>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}
	
	if (tokens.size() > 1)
	{
		string arg = Utils::toLower(tokens[1]);
		if (arg == "help" || arg == "h" || arg == "帮助")
		{
			LOG_INFO(Utils::GetLogger(), "帮助文档 <Recall>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#Recall [回复消息]"));
			return true;
		}
	}
	
	auto quote = gm.GetMessage().GetAll<Mirai::QuoteMessage>();
	if (quote.empty())
	{
		LOG_INFO(Utils::GetLogger(), "格式错误 <Recall>: 未附带回复" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("撤回啥捏"));
		return true;
	}

	try
	{
		client->RecallGroupMessage(quote[0].GetQuoteId(), quote[0].GetGroupId());
	}
	catch (Mirai::MiraiApiHttpException& e)
	{
		LOG_INFO(Utils::GetLogger(), "撤回失败 <Recall>: " + e._message + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("撤回不了捏"));
	}
	return true;
}

}