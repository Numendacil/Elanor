#include <ThirdParty/log.h>
#include <Group/GroupList.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/TriggerStatus.hpp>
#include <libmirai/mirai.hpp>

#include "MorningTrigger.hpp"


namespace Trigger
{

void MorningTrigger::Action()
{
	auto groups_ptr = this->groups.lock();
	if (!groups_ptr)
		return;
	auto group_list = groups_ptr->GetAllGroups();
	Bot::Client& client = Bot::Client::GetClient();
	for (const auto& p : group_list)
	{
		auto enabled = p->GetState<State::TriggerStatus>();
		if (enabled->GetTriggerStatus("Morning"))
		{
			auto GroupInfo = client.GetMiraiClient().GetGroupConfig(p->gid);
			logging::INFO("Send morning <MorningTrigger>\t-> " + GroupInfo.name + "(" + p->gid.to_string() + ")");
			client.SendGroupMessage(p->gid, Mirai::MessageChain().Plain("起床啦！"));
		}
	}
}

}