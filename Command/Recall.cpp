#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include "Recall.hpp"

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool Recall::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() >= std::char_traits<char>::length("#撤回"))
	{
		Utils::ToLower(str);
		Utils::Tokenize(tokens, str);
		if (tokens[0] == "#recall" || tokens[0] == "#撤回")
			return true;
	}
	return false;
}

bool Recall::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	Bot::Client& client = Bot::Client::GetClient();
	logging::INFO("Calling Recall <Recall>" + Utils::GetDescription(gm));
	if (tokens.size() > 1)
	{
		if (tokens[1] == "help" || tokens[1] == "h" || tokens[1] == "帮助")
		{
			logging::INFO("帮助文档 <Recall>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#Recall [回复消息]"));
			return true;
		}
	}
	
	string url = "";
	auto quote = gm.GetMessage().GetAll<Mirai::QuoteMessage>();
	if (quote.empty())
	{
		logging::INFO("格式错误 <Recall>: 未附带回复" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("撤回啥捏"));
		return false;
	}
	try
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		gm.GetMiraiClient().RecallGroupMessage(quote[0].GetQuoteId(), quote[0].GetGroupId());
	}
	catch (Mirai::MiraiApiHttpException& e)
	{
		logging::INFO("撤回失败 <Recall>: " + e._message + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("撤回不了捏"));
		return false;
	}
	return false;
}

}