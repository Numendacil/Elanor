#include "CommandAuth.hpp"

#include <charconv>
#include <string>
#include <string_view>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/Messages/MessageChain.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/AccessCtrlList.hpp>
#include <Core/States/CommandPerm.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>


using std::string;
using std::vector;

namespace GroupCommand
{

bool CommandAuth::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                          Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 2) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#auth" && command != "#权限") return false;


	LOG_INFO(Utils::GetLogger(), "Calling Auth <CommandAuth>" + Utils::GetDescription(gm.GetSender()));
	
	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <CommandAuth>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}


	command = Utils::toLower(tokens[1]);

	if (command == "help" || command == "h" || command == "帮助")
	{
		LOG_INFO(Utils::GetLogger(), "帮助文档 <CommandAuth>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid,
		                        Mirai::MessageChain().Plain(
									"usage:\n#auth set [command] [level]\n#auth {reset/show} [command]\n#auth list"));
		return true;
	}

	auto permission = group.GetState<State::CommandPerm>();
	if (command == "set" || command == "reset" || command == "show" || command == "list")
	{
		if (command == "list")
		{
			auto list = permission->GetCommandList();
			string message = "指令权限列表";
			for (const auto& s : list)
			{
				message += "\n" + s + ": " + std::to_string(permission->GetPermission(s).first);
			}
			LOG_INFO(Utils::GetLogger(), "输出权限列表 <CommandAuth>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(message));
			return true;
		}

		if (tokens.size() < 3)
		{
			LOG_INFO(Utils::GetLogger(),
			         "缺少参数[command] <CommandAuth>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("缺少参数[command]，是被你吃了嘛"));
			return false;
		}

		string target = tokens[2];
		if (!permission->ExistCommand(target))
		{
			LOG_INFO(Utils::GetLogger(),
			         "无效参数[command] <CommandAuth>: " + target + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(target + "是哪个指令捏，不知道捏"));
			return false;
		}

		auto perm = permission->GetPermission(target);

		if (command == "reset")
		{
			permission->UpdatePermission(target, perm.second);
			LOG_INFO(Utils::GetLogger(),
			         "重置权限 <CommandAuth>: " + target + "(" + std::to_string(perm.first) + " -> "
			             + std::to_string(perm.second) + ")" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(
				group.gid,
				Mirai::MessageChain().Plain("重置 " + target + " 的权限等级为 " + std::to_string(perm.second)));
			return true;
		}

		if (command == "show")
		{
			LOG_INFO(Utils::GetLogger(),
			         "输出指令权限 <CommandAuth>: " + target + "(" + std::to_string(perm.first) + ")"
			             + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(
				group.gid, Mirai::MessageChain().Plain(target + " 的权限等级为 " + std::to_string(perm.first)));
			return true;
		}

		if (command == "set")
		{
			if (tokens.size() < 4)
			{
				LOG_INFO(Utils::GetLogger(),
				         "缺少参数[level] <CommandAuth>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("缺少参数[level]，是被你吃了嘛"));
				return true;
			}

			int auth{};
			if (Utils::Str2Num(tokens[3], auth))
			{
				if (auth > 100 || auth < 0) // NOLINT(*-avoid-magic-numbers)
				{
					LOG_INFO(Utils::GetLogger(),
					         "无效参数[level] <CommandAuth>: " + tokens[3]
					             + Utils::GetDescription(gm.GetSender(), false));
					client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限等级必须在0到100之间捏"));
					return true;
				}
			}
			else
			{
				LOG_INFO(Utils::GetLogger(),
				         "无效参数[level] <CommandAuth>: " + tokens[3] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("你觉得" + tokens[3] + "很像个整数么"));
				return true;
			}

			auto ACList = group.GetState<State::AccessCtrlList>();
			// NOLINTNEXTLINE(*-avoid-magic-numbers)
			if ((perm.first >= 50 || auth >= 50)
			    && gm.GetSender().id != ACList->GetSuid())
			{
				LOG_INFO(Utils::GetLogger(),
				         "权限不足 <CommandAuth>: 无法修改该指令的权限(" + target + ")"
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(
					group.gid, Mirai::MessageChain().Plain("你没资格啊，你没资格\n正因如此你没资格啊，你没资格"));
				return true;
			}

			permission->UpdatePermission(target, auth);
			LOG_INFO(Utils::GetLogger(),
			         "更新权限 <CommandAuth>: " + target + "(" + tokens[3] + ")"
			             + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("指令权限更新好了捏"));
			return true;
		}
	}


	LOG_INFO(Utils::GetLogger(), "未知命令 <CommandAuth>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[1] + "是什么指令捏，不知道捏"));
	return true;
}

} // namespace GroupCommand