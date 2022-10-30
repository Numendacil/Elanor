#include "BlackList.hpp"

#include <charconv>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/AccessCtrlList.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

using std::string;
using std::vector;

namespace GroupCommand
{

bool BlackList::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                        Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 2) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#black" && command != "#黑名单" && command != "#blacklist") return false;


	LOG_INFO(Utils::GetLogger(), "Calling BlackList <BlackList>" + Utils::GetDescription(gm.GetSender()));

	command = Utils::toLower(tokens[1]);
	if (command == "help" || command == "h" || command == "帮助")
	{
		LOG_INFO(Utils::GetLogger(), "帮助文档 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid,
		                        Mirai::MessageChain().Plain(
									"usage:\n#blacklist {add/delete/exist} [QQ]...\n#blacklist {clear/clean/list}"));
		return true;
	}

	auto access_list = group.GetState<State::AccessCtrlList>();
	if (command == "clear")
	{
		access_list->BlackListClear();
		LOG_INFO(Utils::GetLogger(), "清除成功 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("黑名单归零了捏"));
		return true;
	}

	if (command == "clean")
	{
		auto list = access_list->GetBlackList();
		std::unordered_set<Mirai::QQ_t> id_list;
		{
			auto member_list = client->GetMemberList(group.gid);
			for (auto&& member : member_list)
				id_list.emplace(member.id);
		}

		for (const auto& id : list)
		{
			if (!id_list.contains(id))
			{
				access_list->BlackListDelete(id);
			}
		}
		LOG_INFO(Utils::GetLogger(), "整理成功 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("黑名单打扫好了捏"));
		return true;
	}

	if (command == "list")
	{
		auto list = access_list->GetBlackList();
		std::unordered_map<Mirai::QQ_t, Mirai::GroupMember> member_map;
		{
			auto member_list = client->GetMemberList(group.gid);
			for (auto&& member : member_list)
				member_map.try_emplace(member.id, std::move(member));
		}

		string msg = "本群黑名单:\n";
		for (const auto& id : list)
		{
			if (member_map.contains(id))
			{
				msg += id.to_string() + " (" + member_map.at(id).MemberName + ")\n";
			}
			else
			{
				msg += id.to_string() + " (目前不在群内)\n";
			}
		}
		LOG_INFO(Utils::GetLogger(), "输出名单 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(msg));
		return true;
	}

	if (command == "exist" || command == "add" || command == "del" || command == "delete")
	{
		auto AtMsg = gm.GetMessage().GetAll<Mirai::AtMessage>();
		if (tokens.size() < 3 && AtMsg.empty())
		{
			LOG_INFO(Utils::GetLogger(),
			         "缺少参数[QQ] <BlackList>: " + command + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("缺少参数[QQ]，是被你吃了嘛"));
			return true;
		}

		vector<Mirai::QQ_t> arr;
		for (int i = 2; i < tokens.size(); i++)
		{
			int64_t id{};
			if (!Utils::Str2Num(tokens[i], id))
			{
				LOG_INFO(Utils::GetLogger(),
				         "无效参数[QQ] <BlackList>: " + tokens[i] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[i] + "是个锤子QQ号"));
				return true;
			}
			arr.emplace_back(id);
		}
		for (const auto& p : AtMsg)
			arr.push_back(p.GetTarget());

		if (command == "exist")
		{
			string msg = "查询结果:\n";
			for (const auto& id : arr)
			{
				msg += id.to_string() + ((access_list->IsBlackList(id)) ? " 在黑名单中\n" : " 不在黑名单中\n");
			}
			LOG_INFO(Utils::GetLogger(), "查询成功 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(msg));
			return true;
		}

		if (command == "add")
		{
			auto botid = client->GetBotQQ();
			for (const auto& id : arr)
				if (id != botid) access_list->BlackListAdd(id);
			LOG_INFO(Utils::GetLogger(), "添加成功 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("黑名单更新好了捏"));
			return true;
		}

		if (command == "delete" || command == "del")
		{
			for (const auto& id : arr)
				access_list->BlackListDelete(id);
			LOG_INFO(Utils::GetLogger(), "删除成功 <BlackList>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("黑名单更新好了捏"));
			return true;
		}
	}

	LOG_INFO(Utils::GetLogger(), "未知命令 <BlackList>: " + command + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(command + "是什么指令捏，不知道捏"));
	return true;
}

} // namespace GroupCommand