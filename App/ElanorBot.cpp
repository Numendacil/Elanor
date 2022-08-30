#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

#include <libmirai/mirai.hpp>

#include <Command/Command.hpp>
#include <Trigger/Trigger.hpp>
#include <Group/GroupList.hpp>
#include <Client/Client.hpp>
#include <Utils/Utils.hpp>
#include <State/AccessCtrlList.hpp>
#include <ThirdParty/log.h>

#include "ElanorBot.hpp"

using std::string;
using std::vector;
using std::pair;
using std::unique_ptr;

namespace Bot
{

vector<pair<string, unique_ptr<GroupCommand::GroupCommandBase>>> RegisterCommands()
{
	vector<pair<string, unique_ptr<GroupCommand::GroupCommandBase>>> v;
	#define REGISTER(_class_) v.push_back(std::move(std::make_pair( string(_class_::_NAME_), std::move(std::make_unique<_class_>()))));

	REGISTER(GroupCommand::WhiteList)
	REGISTER(GroupCommand::BlackList)
	REGISTER(GroupCommand::CommandAuth)
	REGISTER(GroupCommand::SetTrigger)

	REGISTER(GroupCommand::RollDice)
	REGISTER(GroupCommand::Repeat)
	REGISTER(GroupCommand::Recall)
	REGISTER(GroupCommand::AtBot)
	REGISTER(GroupCommand::Bililive)
	REGISTER(GroupCommand::Answer)

	REGISTER(GroupCommand::Petpet)
	REGISTER(GroupCommand::Choyen)
	REGISTER(GroupCommand::ImageSearch)
	REGISTER(GroupCommand::Pixiv)

	REGISTER(GroupCommand::pjskUpdate)
	REGISTER(GroupCommand::pjskSongGuess)
	REGISTER(GroupCommand::pjskCoverGuess)
	REGISTER(GroupCommand::pjskChart)
	REGISTER(GroupCommand::pjskMusicInfo)
	
	REGISTER(GroupCommand::Sing)

	#undef REGISTER

	std::sort(v.begin(), 
		v.end(), 
		[](const pair<string, unique_ptr<GroupCommand::GroupCommandBase>>& a, const pair<string, unique_ptr<GroupCommand::GroupCommandBase>>& b)
		{
			return (a.second)->Priority() > (b.second)->Priority();
		});

	return v;
}

vector<pair<string, unique_ptr<Trigger::TriggerBase>>> RegisterTriggers()
{
	vector<pair<string, unique_ptr<Trigger::TriggerBase>>> v;
	#define REGISTER(_class_) v.push_back(std::move(std::make_pair( string(_class_::_NAME_), std::move(std::make_unique<_class_>()))));

	REGISTER(Trigger::BililiveTrigger)
	REGISTER(Trigger::MorningTrigger)

	#undef REGISTER
	return v;
}

ElanorBot::ElanorBot(Mirai::QQ_t owner_id) :
	is_running(false), suid(owner_id), group_commands(RegisterCommands()), triggers(RegisterTriggers())
{
	vector<pair<string, int>> command_list;
	vector<pair<string, bool>> trigger_list;
	command_list.reserve(this->group_commands.size());
	trigger_list.reserve(this->triggers.size());

	for (const auto& p : this->group_commands)
	{
		command_list.emplace_back(p.first, p.second->Permission());
	}
	for (const auto& p : this->triggers)
	{
		trigger_list.emplace_back(p.first, p.second->TriggerOnStart());
	}

	this->groups = make_shared<GroupList>(owner_id, command_list, trigger_list);
	for (const auto& p : this->triggers)
	{
		p.second->SetGroups(this->groups);
	}
}

void ElanorBot::NudgeEventHandler(Mirai::NudgeEvent& e)
{
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running)	
			return;
	}

	try
	{
		Mirai::MiraiClient& client = e.GetMiraiClient();
		// 如果别人戳机器人，那么就让机器人戳回去
		if (e.GetTarget().GetTarget() != client.GetBotQQ())
			return;
		if (e.GetSender() == client.GetBotQQ())
			return;

		if (e.GetTarget().GetTargetKind() == Mirai::NudgeTarget::GROUP)
		{
			string sender = client.GetMemberInfo(e.GetTarget().GetGroup(), e.GetSender()).MemberName;
			string group_name = client.GetGroupConfig(e.GetTarget().GetGroup()).name;
			logging::INFO("有人戳bot <OnNudgeEvent>\t<- [" + sender + "(" + e.GetSender().to_string() + "), " + group_name + "(" + e.GetTarget().GetGroup().to_string() + ")]");
			
			std::unique_lock<std::mutex> lock(this->nudge_mtx, std::try_to_lock);
			if (!lock)
			{
				logging::INFO("冷却中 <OnNudgeEvent>");
				return;
			}
			
			std::uniform_int_distribution<int> rng05(0, 5);
			int i = rng05(Utils::rng_engine);
			if (i)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				client.SendNudge(Mirai::NudgeTarget{Mirai::NudgeTarget::GROUP, e.GetSender(), e.GetTarget().GetGroup()});
				logging::INFO("戳回去了 <OnNudgeEvent>\t-> [" + sender + "(" + e.GetSender().to_string() + "), " + group_name + "(" + e.GetTarget().GetGroup().to_string() + ")]");
			}
			else
			{
				client.SendGroupMessage(e.GetTarget().GetGroup(), Mirai::MessageChain().At(e.GetSender()).Plain(" 戳你吗"));
				logging::INFO("骂回去了 <OnNudgeEvent>\t-> [" + sender + "(" + e.GetSender().to_string() + "), " + group_name + "(" + e.GetTarget().GetGroup().to_string() + ")]");
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	catch (std::exception &e)
	{
		logging::WARN(e.what());
	}
}

void ElanorBot::GroupMessageEventHandler(Mirai::GroupMessageEvent& gm)
{
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running)	
			return;
	}

	Mirai::MiraiClient& client = gm.GetMiraiClient();
	if (gm.GetSender().id == client.GetBotQQ())
		return;
	
	Group& group = this->groups->GetGroup(gm.GetSender().group.id);
	
	auto access_list = group.GetState<State::AccessCtrlList>();

	if (access_list->IsBlackList(gm.GetSender().id))
		return;

	int auth = 0;
	if (gm.GetSender().id == this->suid)
		auth = 100;
	else if (access_list->IsWhiteList(gm.GetSender().id))
		auth = 50;
	else
	{
		switch (gm.GetSender().permission)
		{
		case Mirai::PERMISSION::MEMBER:
			auth = 0; break;
		case Mirai::PERMISSION::ADMINISTRATOR:
			auth = 10; break;
		case Mirai::PERMISSION::OWNER:
			auth = 20; break; 
		default:
			auth = 0;
		}
	}

	int priority = -1;
	for (const auto& p : this->group_commands)
	{
		if ((p.second)->Priority() < priority)
			break;
		vector<string> token;
		if ((p.second)->Parse(gm.GetMessage(), token))
		{
			if (auth >= p.second->Permission())
			{
				try
				{
					(p.second)->Execute(gm, group, token);
				}
				catch (const std::exception& e)
				{
					logging::ERROR(e.what());
				}
			}
			else
			{
				logging::INFO("权限不足 <OnGroupMessage>: " + ((token.size())? token[0] : string() + Utils::GetDescription(gm, false)));
				client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("权限不足捏~"));
			}
			priority = (p.second)->Priority();
		}
	}
}

void ElanorBot::ConnectionOpenedHandler(Mirai::ClientConnectionEstablishedEvent& e)
{
	logging::INFO("成功建立连接, session-key: " + e.SessionKey);
	this->_run();
}

void ElanorBot::ConnectionClosedHandler(Mirai::ClientConnectionClosedEvent& e)
{
	logging::WARN("连接丢失：" + e.reason + " <" + std::to_string(e.code) + ">");
	this->_stop();
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

void ElanorBot::ConnectionErrorHandler(Mirai::ClientConnectionErrorEvent& e)
{
	logging::WARN("连接时出现错误: " + e.reason + "，重试次数: " + std::to_string(e.RetryCount));
}

void ElanorBot::ParseErrorHandler(Mirai::ClientParseErrorEvent& e)
{
	logging::WARN("解析事件时出现错误: " + string(e.error.what()));
}

}