#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

#include <ThirdParty/log.h>
#include <libmirai/mirai.hpp>
#include <States/States.hpp>

#include "ElanorBot.hpp"
#include "Utils/Common.hpp"

using std::string;
using std::vector;
using std::pair;
using std::unique_ptr;

namespace Bot
{

ElanorBot::ElanorBot(Mirai::QQ_t owner_id) :
	_running(false), _suid(owner_id)
{
}

void ElanorBot::NudgeEventHandler(Mirai::NudgeEvent& e)
{
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		if (!this->_running)	
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
			
			std::unique_lock<std::mutex> lock(this->_NudgeMtx, std::try_to_lock);
			if (!lock)
			{
				logging::INFO("冷却中 <OnNudgeEvent>");
				return;
			}
			
			constexpr int SWORE_PROB = 6;
			std::uniform_int_distribution<int> dist(0, SWORE_PROB - 1);
			int i = dist(Utils::GetRngEngine());
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
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		if (!this->_running)	
			return;
	}

	Mirai::MiraiClient& client = gm.GetMiraiClient();
	if (gm.GetSender().id == client.GetBotQQ())
		return;
	
	Group& group = this->_groups->GetGroup(gm.GetSender().group.id);
	
	auto access_list = group.GetState<State::AccessCtrlList>();

	if (access_list->IsBlackList(gm.GetSender().id))
		return;

	int auth = 0;
	if (gm.GetSender().id == this->_suid)
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
	for (const auto& p : this->_GroupCommands)
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