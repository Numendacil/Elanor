#include <set>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <State/BililiveList.hpp>
#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/GroupList.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/TriggerStatus.hpp>
#include <libmirai/mirai.hpp>

#include "BililiveTrigger.hpp"

using json = nlohmann::json;
using std::string;

namespace Trigger
{

void BililiveTrigger::Action()
{
	auto groups_ptr = this->groups.lock();
	if (!groups_ptr)
		return;
	auto group_list = groups_ptr->GetAllGroups();
	Bot::Client& client = Bot::Client::GetClient();

	std::map<long, std::pair<long, std::set<Mirai::GID_t>>> id_list;	// room_id -> (uid, groups)
	std::map<Mirai::GID_t, State::BililiveList*> state_list;
	for (const auto& p : group_list)
	{
		auto enabled = p->GetState<State::TriggerStatus>();
		if (enabled->GetTriggerStatus("Bililive"))
		{
			auto BiliList = p->GetState<State::BililiveList>();
			state_list.emplace(p->gid, BiliList);
			for (const auto &id : BiliList->GetList())
			{
				id_list[id.second.room_id].first = id.first;
				id_list[id.second.room_id].second.insert(p->gid);
			}
		}
	}

	httplib::Client cli("https://api.live.bilibili.com");
	Utils::SetClientOptions(cli);
	for (const auto& p : id_list)
	{
		long room_id = p.first;
		long uid = p.second.first;
		auto group_set = p.second.second;

		auto result = cli.Get(
			"/room/v1/Room/get_info", {{"id", std::to_string(room_id)}},
			{{"Accept-Encoding", "gzip"}, {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		if (!Utils::CheckHttpResponse(result, "BililiveTrigger: room_info")) continue;

		json content = json::parse(result->body);
		if (content["code"].get<int>() != 0)
		{
			logging::WARN("Error response from /room/v1/Room/get_info <BililiveTrigger>: "
			              + content["msg"].get<string>());
			continue;
		}
		if (content["data"]["live_status"].get<int>() == 1)
		{
			bool AllBroadcasted = true;
			for (const Mirai::GID_t& gid : group_set)
			{
				auto BiliList = state_list.at(gid);
				auto info = BiliList->GetInfo(uid);
				if (!info.broadcasted)
				{
					AllBroadcasted = false;
					break;
				}
			}
			if (AllBroadcasted) continue;

			string title = content["data"]["title"].get<string>();
			string cover = content["data"]["user_cover"].get<string>();
			string area = content["data"]["area_name"].get<string>();
			result = cli.Get(
				"/live_user/v1/Master/info", {{"uid", std::to_string(uid)}},
				{{"Accept-Encoding", "gzip"}, {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
			if (!Utils::CheckHttpResponse(result, "BililiveTrigger: user_info")) continue;

			content = json::parse(result->body);
			if (content["code"].get<int>() != 0)
			{
				logging::WARN("Error response from /live_user/v1/Master/info <BililiveTrigger>: "
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
				auto BiliList = state_list.at(gid);
				auto info = BiliList->GetInfo(uid);
				if (!info.broadcasted)
				{
					BiliList->Broadcast(uid);
					auto GroupInfo = client.GetMiraiClient().GetGroupConfig(gid);
					logging::INFO("发送开播信息 <BililiveTrigger>: " + message + "\t-> " + GroupInfo.name + "("
					              + gid.to_string() + ")");
					client.SendGroupMessage(gid, msg);
				}
			}
		}
		else
		{
			for (const Mirai::GID_t& gid : group_set)
			{
				auto BiliList = state_list.at(gid);
				BiliList->ResetBroadcast(uid);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

}