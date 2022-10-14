
#include "MorningTrigger.hpp"

#include <chrono>
#include <ctime>
#include <thread>

#include <croncpp.h>

#include <libmirai/Client.hpp>
#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Bot/GroupList.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/TriggerStatus.hpp>
#include <Core/Utils/Logger.hpp>


namespace Trigger
{

void MorningTrigger::Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config)
{
	auto group_list = groups.GetAllGroups();
	for (const auto& p : group_list)
	{
		auto enabled = p->GetState<State::TriggerStatus>();
		if (enabled->GetTriggerStatus(std::string(MorningTrigger::_NAME_)))
		{
			auto GroupInfo = client->GetGroupConfig(p->gid);
			LOG_INFO(Utils::GetLogger(),
			         "Send morning <MorningTrigger>" + GroupInfo.name + "(" + p->gid.to_string() + ")");
			client.SendGroupMessage(p->gid, Mirai::MessageChain().Plain("起床啦！"));

			constexpr auto INTERVAL = std::chrono::seconds(5);
			std::this_thread::sleep_for(INTERVAL);
		}
	}
}

time_t MorningTrigger::GetNext()
{
	static const auto crontab = cron::make_cron("0 0 7 * * *");
	return cron::cron_next(crontab, std::time(nullptr));
}

} // namespace Trigger