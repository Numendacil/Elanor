#include "BililiveTrigger.hpp"

#include <chrono>
#include <ctime>
#include <map>
#include <set>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/NetworkUtils.hpp>
#include <State/BililiveList.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <libmirai/Client.hpp>
#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Bot/GroupList.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/CustomState.hpp>
#include <Core/States/TriggerStatus.hpp>
#include <Core/Utils/Logger.hpp>

using json = nlohmann::json;
using std::string;

namespace Trigger
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

void BililiveTrigger::Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config)
{
	std::map<uint64_t, std::pair<uint64_t, std::set<Mirai::GID_t>>> id_list; // room_id -> (uid, groups)
	std::map<Mirai::GID_t, State::BililiveList> state_list;                  // group -> LiveList
	auto group_list = groups.GetAllGroups();

	for (const auto& p : group_list)
	{
		auto enabled = p->GetState<State::TriggerStatus>();
		if (enabled->GetTriggerStatus(string(BililiveTrigger::_NAME_)))
		{
			auto state = p->GetState<State::CustomState>();
			auto bililist =
				state->GetState(string(State::BililiveList::_NAME_), State::BililiveList{}).get<State::BililiveList>();
			state_list.emplace(p->gid, bililist);
			for (const auto& user : bililist.user_list)
			{
				id_list[user.second.room_id].first = user.first;
				id_list[user.second.room_id].second.insert(p->gid);
			}
		}
	}

	httplib::Client cli("https://api.live.bilibili.com");
	SetClientOptions(cli);

	// For each room
	for (const auto& p : id_list)
	{
		uint64_t room_id = p.first;
		uint64_t uid = p.second.first;
		auto group_set = p.second.second;

		// Room info
		auto result = cli.Get(
			"/room/v1/Room/get_info", {{"id", std::to_string(room_id)}},
			{{"Accept-Encoding", "gzip"},
		     {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		if (!Utils::VerifyResponse(result))
		{
			LOG_WARN(Utils::GetLogger(),
			         "Request failed /room/v1/Room/get_info <BililiveTrigger>: "
			             + (result ? httplib::to_string(result.error())
			                       : ("Reason: " + result->reason + ", Body: " + result->body)));
			continue;
		}

		json content = json::parse(result->body);

		if (content["code"].get<int>() != 0)
		{
			LOG_WARN(Utils::GetLogger(),
			         "Error response from /room/v1/Room/get_info <BililiveTrigger>: " + content["msg"].get<string>());
			continue;
		}

		// Is broadcasting
		if (content["data"]["live_status"].get<int>() == 1)
		{
			bool AllBroadcasted = true;
			for (const Mirai::GID_t& gid : group_set)
			{
				State::BililiveList::info& info = state_list.at(gid).user_list.at(uid);
				if (!info.broadcasted)
				{
					AllBroadcasted = false;
					break;
				}
			}
			if (AllBroadcasted) continue;

			// Get live data and user data
			string title = content["data"]["title"].get<string>();
			string cover = content["data"]["user_cover"].get<string>();
			string area = content["data"]["area_name"].get<string>();
			result = cli.Get(
				"/live_user/v1/Master/info", {{"uid", std::to_string(uid)}},
				{{"Accept-Encoding", "gzip"},
			     {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
			if (!Utils::VerifyResponse(result))
			{
				LOG_WARN(Utils::GetLogger(),
				         "Request failed /live_user/v1/Master/info <BililiveTrigger>: "
				             + (result ? httplib::to_string(result.error())
				                       : ("Reason: " + result->reason + ", Body: " + result->body)));
				continue;
			}

			content = json::parse(result->body);
			if (content["code"].get<int>() != 0)
			{
				LOG_WARN(Utils::GetLogger(),
				         "Error response from /live_user/v1/Master/info <BililiveTrigger>: "
				             + content["msg"].get<string>());
				continue;
			}
			string uname = content["data"]["info"]["uname"].get<string>();
			string message = uname + " (" + std::to_string(uid) + ") 正在直播: " + title;
			Mirai::MessageChain msg = Mirai::MessageChain()
										  .Plain(message)
										  .Plain("\n分区: " + area + "\n")
										  .Image("", cover, "", "")
										  .Plain("\nhttps://live.bilibili.com/" + std::to_string(room_id));

			for (const Mirai::GID_t& gid : group_set)
			{
				State::BililiveList::info& info = state_list.at(gid).user_list.at(uid);
				if (!info.broadcasted)
				{
					info.broadcasted = true;
					auto GroupInfo = client->GetGroupConfig(gid);
					LOG_INFO(Utils::GetLogger(),
					         "发送开播信息 <BililiveTrigger>: " + message + "\t-> " + GroupInfo.name + "("
					             + gid.to_string() + ")");
					client.SendGroupMessage(gid, msg);
				}
			}
		}
		else
		{
			for (const Mirai::GID_t& gid : group_set)
			{
				state_list.at(gid).user_list.at(uid).broadcasted = false;
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

time_t BililiveTrigger::GetNext()
{
	using namespace std::chrono;

	constexpr auto interval = minutes(3);

	return system_clock::to_time_t(system_clock::now() + interval);
}

} // namespace Trigger