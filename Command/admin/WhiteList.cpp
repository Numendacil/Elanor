#include <stdexcept>
#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <State/AccessCtrlList.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <libmirai/mirai.hpp>

#include "WhiteList.hpp"

using std::string;
using std::vector;

namespace GroupCommand
{

bool WhiteList::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() > std::char_traits<char>::length("#white"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(tokens, str) < 2)
			return false;
		if (tokens[0] == "#white" || tokens[0] == "#白名单" || tokens[0] == "#whitelist")
			return true;
	}
	return false;
}

bool WhiteList::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 1);
	logging::INFO("Calling WhiteList <WhiteList>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	string command = tokens[1];
	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <WhiteList>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#whitelist {add/delete/exist} [QQ]...\n#whitelist {clear/clean/list}"));
		return true;
	}

	auto access_list = group.GetState<State::AccessCtrlList>();
	if (command == "clear")
	{
		access_list->WhiteListClear();
		logging::INFO("清除成功 <WhiteList>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("白名单归零了捏"));
		return true;
	}

	if (command == "clean")
	{
		auto list = access_list->GetWhiteList();
		for (const auto& id : list)
		{
			try
			{
				(void)gm.GetMiraiClient().GetMemberInfo(gm.GetSender().group.id, id);
			}
			catch (const Mirai::MiraiApiHttpException& e)	// No such member
			{
				access_list->WhiteListDelete(id);
			}
		}
		logging::INFO("整理成功 <WhiteList>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("白名单打扫好了捏"));
		return true;
	}

	if (command == "list")
	{
		auto list = access_list->GetWhiteList();
		string msg = "本群白名单:\n";
		for (const auto& id : list)
		{
			try
			{
				auto profile = gm.GetMiraiClient().GetMemberInfo(gm.GetSender().group.id, id);
				msg += id.to_string() + " (" + profile.MemberName + ")\n";
			}
			catch (const Mirai::MiraiApiHttpException& e)	// No such member
			{
				msg += id.to_string() + " (目前不在群内)\n";
			}
		}
		logging::INFO("输出名单 <WhiteList>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(msg));
		return true;
	}

	if (command == "exist" || command == "add" || command == "del" || command == "delete")
	{
		auto AtMsg = gm.GetMessage().GetAll<Mirai::AtMessage>();
		if (tokens.size() + AtMsg.size() < 3)
		{
			logging::INFO("缺少参数[QQ] <WhiteList>: " + command + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("缺少参数[QQ]，是被你吃了嘛"));
			return false;
		}

		vector<Mirai::QQ_t> arr;
		int i;
		try
		{
			for (i = 2; i < tokens.size(); ++i)
				arr.push_back(Mirai::QQ_t(stol(tokens[i])));
		}
		catch (const std::logic_error& e)
		{
			logging::INFO("无效参数[QQ] <WhiteList>: " + tokens[i] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[i] + "是个锤子QQ号"));
			return false;
		}
		for (const auto& p : AtMsg)
			arr.push_back(p.GetTarget());

		if (command == "exist")
		{
			string msg = "查询结果:\n";
			for (const auto& id : arr)
			{
				msg += id.to_string() + ((access_list->IsWhiteList(id))? " 在白名单中\n" : " 不在白名单中\n");
			}
			logging::INFO("查询成功 <WhiteList>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(msg));
			return true;
		}

		if (command == "add")
		{
			for (const auto& id : arr)
				access_list->WhiteListAdd(id);
			logging::INFO("添加成功 <WhiteList>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("白名单更新好了捏"));
			return true;
		}

		if (command == "delete" || command == "del")
		{
			for (const auto& id : arr)
				access_list->WhiteListDelete(id);
			logging::INFO("删除成功 <WhiteList>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("白名单更新好了捏"));
			return true;
		}
	}

	logging::INFO("未知命令 <WhiteList>: " + command + Utils::GetDescription(gm, false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(command + "是什么指令捏，不知道捏"));
	return false;
}

}