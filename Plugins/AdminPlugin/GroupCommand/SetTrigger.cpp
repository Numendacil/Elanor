#include "SetTrigger.hpp"

#include <stdexcept>
#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/Messages/MessageChain.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/TriggerStatus.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

using std::string;
using std::vector;

namespace GroupCommand
{

bool SetTrigger::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                         Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 2) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#trig" && command != "#trigger" && command != "#触发器") return false;


	LOG_INFO(Utils::GetLogger(), "Calling SetTrigger <SetTrigger>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <SetTrigger>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}

	command = Utils::toLower(tokens[1]);

	if (command == "help" || command == "h" || command == "帮助")
	{
		LOG_INFO(Utils::GetLogger(), "帮助文档 <SetTrigger>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(
			gm.GetSender().group.id,
			Mirai::MessageChain().Plain(
				"usage:\n#trigger set [trigger] {on/off}\n#trigger status [trigger]\n#trigger list"));
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
				message += "\n" + s + ": " + ((trigger_status->GetTriggerStatus(s)) ? "✅" : "❌");
			}
			LOG_INFO(Utils::GetLogger(), "输出触发器列表 <SetTrigger>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(message));
			return true;
		}

		if (tokens.size() < 3)
		{
			LOG_INFO(Utils::GetLogger(),
			         "缺少参数[trigger] <SetTrigger>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(gm.GetSender().group.id,
			                        Mirai::MessageChain().Plain("缺少参数[trigger]，是被你吃了嘛"));
			return true;
		}

		string target = tokens[2];
		if (!trigger_status->ExistTrigger(target))
		{
			LOG_INFO(Utils::GetLogger(),
			         "无效参数[trigger] <SetTrigger>: " + target + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(gm.GetSender().group.id,
			                        Mirai::MessageChain().Plain(target + "是哪个触发器捏，不知道捏"));
			return true;
		}

		if (command == "status")
		{
			LOG_INFO(Utils::GetLogger(),
			         "输出触发器状态 <SetTrigger>: " + target
			             + string((trigger_status->GetTriggerStatus(target)) ? "✅" : "❌")
			             + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(
				gm.GetSender().group.id,
				Mirai::MessageChain().Plain(
					target + " 当前状态: " + string((trigger_status->GetTriggerStatus(target)) ? "✅" : "❌")));
			return true;
		}

		if (tokens.size() < 4)
		{
			LOG_INFO(Utils::GetLogger(),
			         "缺少参数{on/off} <SetTrigger>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(
				gm.GetSender().group.id,
				Mirai::MessageChain().Plain("on还是off, 到底要选哪个呢🔉到底要选哪个呢🔉到底要选哪个呢🔉"));
			return true;
		}

		try
		{
			string status = Utils::toLower(tokens[3]);
			if (Utils::toBool(status))
			{
				trigger_status->UpdateTriggerStatus(target, true);
				LOG_INFO(Utils::GetLogger(),
				         "Trigger on ✅ <SetTrigger>: " + target + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(gm.GetSender().group.id,
				                        Mirai::MessageChain().Plain("已启动 " + target + " ✅"));
				return true;
			}
			else
			{
				trigger_status->UpdateTriggerStatus(target, false);
				LOG_INFO(Utils::GetLogger(),
				         "Trigger off ❌ <SetTrigger>: " + target + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(gm.GetSender().group.id,
				                        Mirai::MessageChain().Plain("已关闭 " + target + " ❌"));
				return true;
			}
		}
		catch (const Utils::UnknownInput&)
		{
			LOG_INFO(Utils::GetLogger(),
			         "未知选项 <SetTrigger>: " + tokens[3] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(gm.GetSender().group.id,
			                        Mirai::MessageChain().Plain(tokens[3] + "是什么意思捏，看不懂捏"));
			return true;
		}
	}


	LOG_INFO(Utils::GetLogger(), "未知指令 <SetTrigger>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[1] + "是什么东西捏，不知道捏"));
	return true;
}

} // namespace GroupCommand