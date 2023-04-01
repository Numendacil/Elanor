#include <ThirdParty/log.h>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/TriggerStatus.hpp>
#include <Utils/Utils.hpp>

#include "SetTrigger.hpp"

using std::string;
using std::vector;

namespace GroupCommand
{

bool SetTrigger::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() > std::char_traits<char>::length("#trig"))
	{
		if (Utils::Tokenize(tokens, str) < 2)
			return false;
		Utils::ToLower(tokens[0]);
		if (tokens[0] == "#trig" || tokens[0] == "#trigger" || tokens[0] == "#触发器")
			return true;
	}
	return false;
}

bool SetTrigger::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 1);
	logging::INFO("Calling SetTrigger <SetTrigger>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	string command = tokens[1];
	Utils::ToLower(command);

	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <SetTrigger>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#trigger set [trigger] {on/off}\n#trigger status [trigger]\n#trigger list"));
		return true;
	}

	auto trigger_status = group.GetState<State::TriggerStatus>();

	if (command == "set" || command == "status" || command == "list")
	{
		if (command == "list")
		{
			auto list = trigger_status->GetTriggerList();
			string message = "触发器列表";
			for (const auto& s : list)
			{
				message += "\n" + s + ": " + ((trigger_status->GetTriggerStatus(s))? "✅" : "❌");
			}
			logging::INFO("输出触发器列表 <SetTrigger>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(message));
			return true;
		}

		if (tokens.size() < 3)
		{
			logging::INFO("缺少参数[trigger] <SetTrigger>: " + tokens[1] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("缺少参数[trigger]，是被你吃了嘛"));
			return false;
		}

		string target = tokens[2];
		if (!trigger_status->ExistTrigger(target))
		{
			logging::INFO("无效参数[trigger] <SetTrigger>: " + target + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(target + "是哪个触发器捏，不知道捏"));
			return false;
		}

		if (command == "status")
		{
			logging::INFO("输出触发器状态 <SetTrigger>: " + target + string((trigger_status->GetTriggerStatus(target))? "✅" : "❌") + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(target + " 当前状态: " + string((trigger_status->GetTriggerStatus(target))? "✅" : "❌")));
			return true;
		}

		if (tokens.size() < 4)
		{
			logging::INFO("缺少参数{on/off} <SetTrigger>: " + tokens[1] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("on还是off, 到底要选哪个呢🔉到底要选哪个呢🔉到底要选哪个呢🔉"));
			return false;
		}

		switch(Utils::ToBool(tokens[3]))
		{
		case 1:
			trigger_status->UpdateTriggerStatus(target, true);
			logging::INFO("Trigger on ✅ <SetTrigger>: " + target + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("已启动 " + target + " ✅"));
			return true;

		case 0:
			trigger_status->UpdateTriggerStatus(target, false);
			logging::INFO("Trigger off ❌ <SetTrigger>: " + target + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("已关闭 " + target + " ❌"));
			return true;

		default:
			logging::INFO("未知选项 <SetTrigger>: " + tokens[3] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[3] + "是什么意思捏，看不懂捏"));
			return false;
		}
	}


	logging::INFO("未知指令 <SetTrigger>: " + tokens[1] + Utils::GetDescription(gm, false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[1] + "是什么东西捏，不知道捏"));
	return false;
}

}