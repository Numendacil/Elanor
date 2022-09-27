#include <string>
#include <Utils/Utils.hpp>
#include <ThirdParty/log.h>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/CommandPerm.hpp>

#include "CommandAuth.hpp"


using std::string;
using std::vector;

namespace GroupCommand
{

bool CommandAuth::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() > std::char_traits<char>::length("#auth"))
	{
		if (Utils::Tokenize(tokens, str) < 2)
			return false;
		Utils::ToLower(tokens[0]);
		if (tokens[0] == "#auth" || tokens[0] == "#权限")
			return true;
	}
	return false;
}

bool CommandAuth::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 1);
	logging::INFO("Calling Auth <CommandAuth>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	string command = tokens[1];
	Utils::ToLower(command);

	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <CommandAuth>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#auth set [command] [level]\n#auth {reset/show} [command]\n#auth list"));
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
			logging::INFO("输出权限列表 <CommandAuth>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(message));
			return true;
		}

		if (tokens.size() < 3)
		{
			logging::INFO("缺少参数[command] <CommandAuth>: " + tokens[1] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("缺少参数[command]，是被你吃了嘛"));
			return false;
		}
		
		string target = tokens[2];
		if (!permission->ExistCommand(target))
		{
			logging::INFO("无效参数[command] <CommandAuth>: " + target + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(target + "是哪个指令捏，不知道捏"));
			return false;
		}

		auto perm = permission->GetPermission(target);

		if (command == "reset")
		{
			permission->UpdatePermission(target, perm.second);
			logging::INFO("重置权限 <CommandAuth>: " + target + "(" + std::to_string(perm.first) + " -> " + std::to_string(perm.second) + ")" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("重置 " + target + " 的权限等级为 " + std::to_string(perm.second)));
			return true;
		}

		if (command == "show")
		{
			logging::INFO("输出指令权限 <CommandAuth>: " + target + "(" +std:: to_string(perm.first) + ")" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(target + " 的权限等级为 " + std::to_string(perm.first)));
			return true;
		}

		// set

		if (tokens.size() < 4)
		{
			logging::INFO("缺少参数[level] <CommandAuth>: " + tokens[1] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("缺少参数[level]，是被你吃了嘛"));
			return false;
		}

		int auth;
		try
		{
			auth = stoi(tokens[3]);
			if (auth > 100 || auth < 0)
			{
				logging::INFO("无效参数[level] <CommandAuth>: " + tokens[3] + Utils::GetDescription(gm, false));
				client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("权限等级必须在0到100之间捏"));
				return false;
			}
		}
		catch(const std::logic_error& e)
		{
			logging::INFO("无效参数[level] <CommandAuth>: " + tokens[3] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("你觉得" + tokens[3] + "很像个整数么"));
			return false;
		}

		if ((perm.first >= 50 || auth >= 50) && gm.GetSender().id != group.suid)
		{
			logging::INFO("权限不足 <CommandAuth>: 无法修改该指令的权限(" + target + ")" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("你没资格啊，你没资格\n正因如此你没资格啊，你没资格"));
			return false;
		}

		permission->UpdatePermission(target, auth);
		logging::INFO("更新权限 <CommandAuth>: " + target + "(" + tokens[3] + ")" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("指令权限更新好了捏"));
		return true;
	}


	logging::INFO("未知命令 <CommandAuth>: " + tokens[1] + Utils::GetDescription(gm, false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[1] + "是什么指令捏，不知道捏"));
	return false;
}

}