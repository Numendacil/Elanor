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
	if (this->_streams.empty())
	{
		std::map<uint64_t, std::pair<uint64_t, std::set<Mirai::GID_t>>> id_list;
		auto group_list = groups.GetAllGroups();
		for (const auto& p : group_list)
		{
			auto TriggerStatus = p->GetState<State::TriggerStatus>();
			if (TriggerStatus->GetTriggerStatus(string(BililiveTrigger::_NAME_)))
			{
				auto state = p->GetState<State::CustomState>();
				auto bililist = state->GetState(
					State::BililiveList::_NAME_.data(), 
					State::BililiveList{}
				).get<State::BililiveList>();

				for (const auto& user : bililist.user_list)
				{
					id_list[user.second.room_id].first = user.first;
					id_list[user.second.room_id].second.insert(p->gid);
				}
			}
		}

		for (auto&& [RoomId, value] : id_list)
			this->_streams.emplace(RoomId, value.first, std::move(value.second));

		if (this->_streams.empty())
			return;
	}

	auto stream = std::move(this->_streams.front());
	this->_streams.pop();

	httplib::Client cli("https://api.live.bilibili.com");
	SetClientOptions(cli);

	// Room info
	auto result = cli.Get(
		"/room/v1/Room/get_info", 
		{{"id", std::to_string(stream.RoomId)}},
		{
			{"Accept-Encoding", "gzip, deflate"},
			{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
		}
	);
	if (!Utils::VerifyResponse(result))
	{
		LOG_WARN(Utils::GetLogger(),
			"Request failed /room/v1/Room/get_info <BililiveTrigger>: "
			+ (result ? httplib::to_string(result.error())
			: ("Reason: " + result->reason + ", Body: " + result->body)));
		return;
	}

	json content = json::parse(result->body);

	if (content["code"].get<int>() != 0)
	{
		LOG_WARN(Utils::GetLogger(),
			"Error response from /room/v1/Room/get_info <BililiveTrigger>: " + content["msg"].get<string>());
		return;
	}

	std::map<Mirai::GID_t, State::CustomState*> StateList;
	for (const Mirai::GID_t& gid : stream.groups)
		StateList.emplace(gid, groups.GetGroup(gid).GetState<State::CustomState>());

	// Is broadcasting
	if (content["data"]["live_status"].get<int>() == 1)
	{
		bool AllBroadcasted = true;

		for (const Mirai::GID_t& gid : stream.groups)
		{
			const auto& info = StateList.at(gid)->GetState(
				State::BililiveList::_NAME_.data(), 
				State::BililiveList{}
			).get<State::BililiveList>().user_list.at(stream.uid);
			if (!info.broadcasted)
			{
				AllBroadcasted = false;
				break;
			}
		}
		if (AllBroadcasted) 
			return;

		// Get live data and user data
		string title = content["data"]["title"].get<string>();
		string cover = content["data"]["user_cover"].get<string>();
		string area = content["data"]["area_name"].get<string>();
		result = cli.Get(
			"/live_user/v1/Master/info", 
			{{"uid", std::to_string(stream.uid)}},
			{
				{"Accept-Encoding", "gzip, deflate"},
				{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
			}
		);
		if (!Utils::VerifyResponse(result))
		{
			LOG_WARN(Utils::GetLogger(),
				"Request failed /live_user/v1/Master/info <BililiveTrigger>: "
				+ (result ? httplib::to_string(result.error())
				: ("Reason: " + result->reason + ", Body: " + result->body)));
			return;
		}

		content = json::parse(result->body);
		if (content["code"].get<int>() != 0)
		{
			LOG_WARN(Utils::GetLogger(),
					"Error response from /live_user/v1/Master/info <BililiveTrigger>: "
					+ content["msg"].get<string>());
			return;
		}

		string uname = content["data"]["info"]["uname"].get<string>();
		string message = uname + " (" + std::to_string(stream.uid) + ") 正在直播: " + title;
		Mirai::MessageChain msg = Mirai::MessageChain()
					.Plain(message)
					.Plain("\n分区: " + area + "\n")
					.Image("", cover, "", "")
					.Plain("\nhttps://live.bilibili.com/" + std::to_string(stream.RoomId));

		for (const Mirai::GID_t& gid : stream.groups)
		{
			auto bililist = StateList.at(gid)->GetState(
				State::BililiveList::_NAME_.data(), 
				State::BililiveList{}
			).get<State::BililiveList>();

			auto& info = bililist.user_list.at(stream.uid);
			if (!info.broadcasted)
			{
				info.broadcasted = true;
				StateList.at(gid)->SetState(
					State::BililiveList::_NAME_.data(),
					bililist
				);

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
		for (const Mirai::GID_t& gid : stream.groups)
		{
			auto bililist = StateList.at(gid)->GetState(
				State::BililiveList::_NAME_.data(), 
				State::BililiveList{}
			).get<State::BililiveList>();
			State::BililiveList::info& info = bililist.user_list.at(stream.uid);
			if (info.broadcasted)
			{
				info.broadcasted = false;
				StateList.at(gid)->SetState(
					State::BililiveList::_NAME_.data(),
					bililist
				);
			}
		}
	}
}

time_t BililiveTrigger::GetNext()
{
	using namespace std::chrono;
	using namespace std::literals;
	constexpr auto interval = 20s;

	return system_clock::to_time_t(system_clock::now() + interval);
}

} // namespace Trigger