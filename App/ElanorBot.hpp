#ifndef _ELANOR_BOT_HPP_
#define _ELANOR_BOT_HPP_

#include <mutex>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include <Command/GroupCommandBase.hpp>
#include <Trigger/TriggerBase.hpp>
#include <Group/GroupList.hpp>

#include <libmirai/mirai.hpp>

namespace Bot
{

class Group;

class ElanorBot
{

protected:
	mutable std::mutex mtx;
	mutable std::mutex nudge_mtx;

	template <typename T>
	using Tag = std::pair<std::string, std::unique_ptr<T>>;
	const std::vector<Tag<GroupCommand::GroupCommandBase>> group_commands;
	const std::vector<Tag<Trigger::TriggerBase>> triggers;
	
	std::shared_ptr<GroupList> groups;

	bool is_running;

	const Mirai::QQ_t suid;

	void _run()
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->is_running) return;
		this->is_running = true;
		for (const auto& p : this->triggers)
			p.second->TriggerOn();
	}
	void _stop()
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running) return;
		this->is_running = false;
		for (const auto& p : this->triggers)
			p.second->TriggerOff();
	}

public:
	ElanorBot(Mirai::QQ_t owner_id);

	void NudgeEventHandler(Mirai::NudgeEvent& e);
	void GroupMessageEventHandler(Mirai::GroupMessageEvent& gm);
	void ConnectionOpenedHandler(Mirai::ClientConnectionEstablishedEvent& e);
	void ConnectionClosedHandler(Mirai::ClientConnectionClosedEvent& e);
	void ConnectionErrorHandler(Mirai::ClientConnectionErrorEvent& e);
	void ParseErrorHandler(Mirai::ClientParseErrorEvent& e);

	~ElanorBot()
	{
		this->_stop();
	}
};

}

#endif