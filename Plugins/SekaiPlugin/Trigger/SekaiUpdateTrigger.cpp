#include "SekaiUpdateTrigger.hpp"

#include <chrono>
#include <exception>
#include <regex>
#include <vector>
#include <croncpp.h>

#include <SekaiClient/SekaiClient.hpp>
#include <SekaiClient/Singleton.hpp>

#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>
#include <Core/Bot/GroupList.hpp>
#include <Core/States/TriggerStatus.hpp>
#include <Core/Client/Client.hpp>

#include <libmirai/mirai.hpp>

using std::string;

using namespace std::literals;

namespace Trigger
{

namespace
{

Mirai::MessageChain GetUpdatedVersions(Sekai::SekaiClient& SekaiCli, Bot::Client& /*client*/, const std::vector<string>& /*UpdatedContents*/)
{
	string AppVersion;
	string AssetVersion;
	string DataVersion;
	{
		auto versions = SekaiCli.GetMetaInfo("versions");
		for (const auto& obj : versions.at("appVersions"))
		{
			if (obj.at("appVersionStatus").get<string>() == "available")
			{
				AppVersion = obj.at("appVersion").get<string>();
				AssetVersion = obj.at("assetVersion").get<string>();
				DataVersion = obj.at("dataVersion").get<string>();
				break;
			}
		}
	}

	return Mirai::MessageChain().Plain(
		"Project Sekai 有新的更新: \nApp Ver. "
		+ AppVersion
		+ "\nAsset Ver. " + AssetVersion
		+ "\nData Ver." + DataVersion
	);
}

Mirai::MessageChain GetUpdatedCards(Sekai::SekaiClient& SekaiCli, Bot::Client& client, const std::vector<string>& UpdatedContents)
{
	const static std::regex reg(R"(^character/member/res\d+_no\d+$)", std::regex_constants::ECMAScript);

	Mirai::ForwardMessage msg;
	Mirai::ForwardMessage::Node node;
	node.SetSenderId(client->GetBotQQ());
	node.SetSenderName("sbga");

	for (const auto& key : UpdatedContents)
	{
		if (!std::regex_match(key, reg))
			continue;

		auto contents = SekaiCli.GetAssetContents(key);
		for (const auto& file : contents)
		{
			if (file.extension() == ".png")
			{
				auto image = SekaiCli.GetFileContents(key, file);
				node.SetTimestamp(std::time(nullptr));
				node.SetMessageChain(Mirai::MessageChain().Image(client->UploadGroupImage(std::move(image))));
				msg.emplace_back(node);
			}
		}
	}

	return Mirai::MessageChain().Forward(std::move(msg));
}

}

void SekaiUpdateTrigger::Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config)
{
	auto SekaiCli = Sekai::GetClient(config);

	std::vector<string> UpdatedContents;
	try
	{
		UpdatedContents = SekaiCli->UpdateContents();
	}
	catch(const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(), "Failed to fetch updates <SekaiUpdateTrigger>: "s + e.what());
		return;
	}

	if (UpdatedContents.empty())
		return;

	std::vector<Mirai::GID_t> EnabledGroups;
	{
		auto GroupList = groups.GetAllGroups();
		for (const auto& p : GroupList)
		{
			auto enabled = p->GetState<State::TriggerStatus>();
			if (enabled->GetTriggerStatus(std::string(SekaiUpdateTrigger::_NAME_)))
			{
				EnabledGroups.push_back(p->gid);
			}
		}
	}

	if (EnabledGroups.empty())
		return;

	LOG_INFO(Utils::GetLogger(), "Preparing update messages");
	auto versions = GetUpdatedVersions(*SekaiCli, client, UpdatedContents);
	auto cards = GetUpdatedCards(*SekaiCli, client, UpdatedContents);

	for (const auto& gid : EnabledGroups)
	{
		auto GroupInfo = client->GetGroupConfig(gid);
		LOG_INFO(Utils::GetLogger(), "发送更新信息 <SekaiUpdateTrigger>\t-> " 
			+ GroupInfo.name + "("
			+ gid.to_string() + ")");
		client->SendGroupMessage(gid, versions);
		client->SendGroupMessage(gid, cards);
	}

	
}

time_t SekaiUpdateTrigger::GetNext()
{
	static const auto crontab = cron::make_cron("1 1/10 * * * *");
	return cron::cron_next(crontab, std::time(nullptr));
}

} // namespace Trigger