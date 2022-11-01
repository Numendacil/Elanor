#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>
#include <charconv>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

#include <Core/States/CoolDown.hpp>

#include <Handlers/SauceSearch.hpp>

#include "ImageSearch.hpp"

using std::string;
using std::vector;
using namespace std::literals;

namespace GroupCommand
{

bool ImageSearch::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 1) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#search" && command !=  "#搜图") return false;


	LOG_INFO(Utils::GetLogger(), "Calling ImageSearch <ImageSearch>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <ImageSearch>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}

	enum SERVER_TYPE
	{ 
		SAUCENAO, 
		// ASCII2D, 
		// TRACEMOE 
	} server = SAUCENAO;
	if (tokens.size() > 1)
	{
		string arg = Utils::toLower(tokens[1]);
		if (arg == "help" || arg == "h" || arg == "帮助")
		{
			LOG_INFO(Utils::GetLogger(), "帮助文档 <ImageSearch>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("usage:\n#search (server) [图片]"));
			return true;
		}

		else if (arg == "saucenao" || arg == "sauce")
		{
			server = SAUCENAO;
		}

		// else if (arg == "ascii2d" || arg == "ascii")
		// {
		// 	server = ASCII2D;
		// }

		// else if (arg == "trace" || arg == "moe" || arg == "tracemoe")
		// {
		// 	server = TRACEMOE;
		// }

		else
		{
			LOG_INFO(Utils::GetLogger(), "未知搜索引擎 <ImageSearch>: " + arg + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(arg + "是什么搜索引擎捏，不知道捏"));
			return true;
		}
	}
	
	LOG_INFO(Utils::GetLogger(), "搜索引擎 <ImageSearch>: " + std::to_string(server) + Utils::GetDescription(gm.GetSender(), false));

	string url = "";
	auto img = gm.GetMessage().GetAll<Mirai::ImageMessage>();
	if (img.size())
	{
		url = img[0].GetImage().url;
	}
	else
	{
		auto quote = gm.GetMessage().GetAll<Mirai::QuoteMessage>();
		if (quote.empty())
		{
			LOG_INFO(Utils::GetLogger(), "格式错误 <ImageSearch>: 未附带图片或回复" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("图捏"));
			return true;
		}
		try
		{
			Mirai::GroupMessageEvent quote_gm = gm.GetMiraiClient().GetGroupMessage(quote[0].GetQuoteId(), quote[0].GetGroupId());
			img = quote_gm.GetMessage().GetAll<Mirai::ImageMessage>();
			if (img.size())
			{
				url = img[0].GetImage().url;
			}
			else
			{
				LOG_INFO(Utils::GetLogger(), "格式错误 <ImageSearch>: 回复内容不包含图片" + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("图捏"));
				return true;
			}
		}
		catch (const Mirai::NoElement &)
		{
			LOG_INFO(Utils::GetLogger(), "无法从回复内容获得图片 <ImageSearch>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("消息太久了，看不到是什么图捏"));
			return true;
		}
	}
	LOG_INFO(Utils::GetLogger(), "图片链接 <ImageSearch>: " + url);

	switch(server)
	{
	case SAUCENAO:
		SearchHandlers::SearchSauce(std::move(url), gm, group, client, config);
		break;
	// case ASCII2D:
	// 	cooldown = 60s;
	// 	path = "ascii2d/";
	// 	break;
	// case TRACEMOE:
	// 	cooldown = 60s;
	// 	path = "trace-moe/";
	// 	break;

	default:	// You should never be here
		LOG_ERROR(Utils::GetLogger(), "waht");
	}
	return true;
}

}