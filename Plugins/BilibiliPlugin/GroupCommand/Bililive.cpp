#include "Bililive.hpp"

#include <charconv>
#include <exception>
#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/NetworkUtils.hpp>
#include <PluginUtils/StringUtils.hpp>
#include <State/BililiveList.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/CustomState.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

namespace
{

void SetClientOptions(httplib::Client& cli)
{
	cli.set_compress(true);
	cli.set_decompress(true);
	cli.set_connection_timeout(300); // NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(300);       // NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(120);      // NOLINT(*-avoid-magic-numbers)
	cli.set_keep_alive(true);
}

} // namespace

bool Bililive::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                       Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 2) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#live" && command != "#直播") return false;


	LOG_INFO(Utils::GetLogger(), "Calling Bililive <Bililive>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <Bililive>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}


	command = Utils::toLower(tokens[1]);

	if (command == "help" || command == "h" || command == "帮助")
	{
		LOG_INFO(Utils::GetLogger(), "帮助文档 <Bililive>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid,
		                        Mirai::MessageChain().Plain("usage:\n#live add [uid]\n#live del [uid]\n#live list"));
		return true;
	}

	httplib::Client cli("https://api.live.bilibili.com");
	SetClientOptions(cli);

	auto state = group.GetState<State::CustomState>();
	auto bililist =
		state->GetState(string(State::BililiveList::_NAME_), State::BililiveList{}).get<State::BililiveList>();

	if (command == "list")
	{
		string message = "直播间列表: ";
		for (const auto& [uid, info] : bililist.user_list)
		{
			auto result = cli.Get(
				"/live_user/v1/Master/info", 
				{{"uid", std::to_string(uid)}},
				{
					{"Accept-Encoding", "gzip, deflate"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
				}
			);

			json content;
			try
			{
				content = Utils::GetJsonResponse(result);
			}
			catch (const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(),
				         "Request failed /live_user/v1/Master/info <Bililive>: " + string(e.what()));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}

			if (content["code"].get<int>() != 0)
			{
				LOG_WARN(Utils::GetLogger(),
				         "Error response from /live_user/v1/Master/info <Bililive>: " + content["msg"].get<string>());
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}
			message += "\n" + content["data"]["info"]["uname"].get<string>() + " (" + std::to_string(uid) + "): ";

			result = cli.Get(
				"/room/v1/Room/get_info", 
				{{"id", std::to_string(info.room_id)}},
				{
					{"Accept-Encoding", "gzip, deflate"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
				}
			);

			try
			{
				content = Utils::GetJsonResponse(result);
			}
			catch (const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(), "Request failed /room/v1/Room/get_info <Bililive>: " + string(e.what()));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}

			if (content["code"].get<int>() != 0)
			{
				LOG_WARN(Utils::GetLogger(),
				         "Error response from /room/v1/Room/get_info <Bililive>: " + content["msg"].get<string>());
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}
			if (content["data"]["live_status"].get<int>() == 0) 
				message += "未开播 ⚫";
			else
				message += (content["data"]["live_status"].get<int>() == 1) ? "直播中 🔴" : "轮播中 🔵";

			constexpr auto interval = std::chrono::milliseconds(200);
			std::this_thread::sleep_for(interval);
		}
		LOG_INFO(Utils::GetLogger(), "输出直播间列表 <Bililive>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(message));
		return true;
	}

	if (command == "add" || command == "del")
	{
		if (tokens.size() < 3)
		{
			LOG_INFO(Utils::GetLogger(),
			         "缺少参数[uid] <Bililive>: " + command + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("缺少参数[uid]，是被你吃了嘛"));
			return true;
		}

		long uid{};
		if (!Utils::Str2Num(tokens[2], uid))
		{
			LOG_INFO(Utils::GetLogger(),
			         "无效参数[uid] <Bililive>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[2] + "是个锤子uid"));
			return true;
		}

		if (command == "add")
		{
			if (bililist.user_list.contains(uid))
			{
				LOG_INFO(Utils::GetLogger(),
				         "用户已存在 <Bililive>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该用户已经在名单里了捏"));
				return true;
			}

			auto result = cli.Get(
				"/live_user/v1/Master/info", 
				{{"uid", std::to_string(uid)}}, 
				{
					{"Accept-Encoding", "gzip, deflate"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
				}
			);

			json content;
			try
			{
				content = Utils::GetJsonResponse(result);
			}
			catch (const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(), "Request failed /live_user/v1/Master/info <Bililive>: " + string(e.what()));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}

			if (content["code"].get<int>() != 0)
			{
				LOG_WARN(Utils::GetLogger(),
					"Error response from /live_user/v1/Master/info' <Bililive>: " + content["msg"].get<string>());
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}
			if (content["data"]["info"]["uname"].get<string>().empty())
			{
				LOG_INFO(Utils::GetLogger(),
					"用户不存在 <Bililive>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该用户不存在捏"));
				return true;
			}
			if (content["data"]["room_id"].get<long>() == 0)
			{
				LOG_INFO(Utils::GetLogger(),
					"直播间不存在 <Bililive>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该用户貌似暂未开通直播功能捏"));
				return true;
			}


			long room_id = content["data"]["room_id"].get<long>();
			string pic = content["data"]["info"]["face"].get<string>();
			string name = content["data"]["info"]["uname"].get<string>();

			state->ModifyState(string(State::BililiveList::_NAME_),
			                   [&uid, &room_id](json& state)
			                   {
								   auto bililist = state.get<State::BililiveList>();
								   bililist.user_list.try_emplace(uid, room_id, false);
								   state = bililist;
							   });

			LOG_INFO(Utils::GetLogger(),
			         "成功添加用户 <Bililive>: " + name + "(" + std::to_string(uid) + ")"
			             + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid,
			                        Mirai::MessageChain()
			                            .Plain("成功添加用户" + name + "(" + std::to_string(uid) + ")\n")
			                            .Image("", pic, "", ""));
			return true;
		}

		if (command == "del")
		{
			if (!bililist.user_list.contains(uid))
			{
				LOG_INFO(Utils::GetLogger(),
				         "用户不存在 <Bililive>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该用户还不在名单里捏"));
				return true;
			}

			auto result = cli.Get(
				"/live_user/v1/Master/info", 
				{{"uid", std::to_string(uid)}}, 
				{
					{"Accept-Encoding", "gzip, deflate"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
				}
			);

			json content;
			try
			{
				content = Utils::GetJsonResponse(result);
			}
			catch (const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(), "Request failed /live_user/v1/Master/info <Bililive>: " + string(e.what()));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}

			if (content["code"].get<int>() != 0)
			{
				LOG_WARN(Utils::GetLogger(),
					"Error response from /live_user/v1/Master/info' <Bililive>: " + content["msg"].get<string>());
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return true;
			}
			if (content["data"]["info"]["uname"].get<string>().empty())
			{
				LOG_INFO(Utils::GetLogger(),
					"用户不存在 <Bililive>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该用户不存在捏"));
				return true;
			}

			string pic = content["data"]["info"]["face"].get<string>();
			string name = content["data"]["info"]["uname"].get<string>();

			state->ModifyState(string(State::BililiveList::_NAME_),
			                   [&uid](json& state)
			                   {
								   auto bililist = state.get<State::BililiveList>();
								   bililist.user_list.erase(uid);
								   state = bililist;
							   });

			LOG_INFO(Utils::GetLogger(),
			         "成功删除用户 <Bililive>: " + name + "(" + std::to_string(uid) + ")"
			             + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid,
			                        Mirai::MessageChain()
			                            .Plain("成功删除用户" + name + "(" + std::to_string(uid) + ")\n")
			                            .Image("", pic, "", ""));
			return true;
		}
	}


	LOG_INFO(Utils::GetLogger(), "未知命令 <Bililive>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[1] + "是什么指令捏，不知道捏"));
	return true;
}

} // namespace GroupCommand