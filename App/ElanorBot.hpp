#ifndef _ELANOR_BOT_HPP_
#define _ELANOR_BOT_HPP_

#include <mutex>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include <Interface/IGroupCommand.hpp>
#include <Interface/ITrigger.hpp>
#include <Bot/GroupList.hpp>

#include <libmirai/Events/Events.hpp>
#include <libmirai/Types/BasicTypes.hpp>

namespace Bot
{

class GroupList;
class Client;

class ElanorBot
{
protected:
	mutable std::mutex _MemberMtx;
	mutable std::mutex _NudgeMtx;

	template <typename T>
	using Tag = std::pair<std::string, std::unique_ptr<T>>;
	std::vector<Tag<GroupCommand::IGroupCommand>> _GroupCommands;
	std::vector<Tag<Trigger::ITrigger>> _triggers;
	
	std::shared_ptr<GroupList> _groups;
	std::shared_ptr<Client> _client;

	bool _running;

	const Mirai::QQ_t _suid;

	void _run()
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		if (this->_running) return;
		this->_running = true;
		for (const auto& p : this->_triggers)
			p.second->TriggerOn();
	}
	void _stop()
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		if (!this->_running) return;
		this->_running = false;
		for (const auto& p : this->_triggers)
			p.second->TriggerOff();
	}

public:
	ElanorBot(Mirai::QQ_t owner_id);
	ElanorBot(const ElanorBot&) = delete;
	ElanorBot& operator=(const ElanorBot&) = delete;
	ElanorBot(ElanorBot&&) = delete;
	ElanorBot& operator=(ElanorBot&&) = delete;

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