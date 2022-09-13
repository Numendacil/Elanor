#ifndef _ELANOR_CORE_TRIGGER_INTERFACE_HPP_
#define _ELANOR_CORE_TRIGGER_INTERFACE_HPP_

#include <string>
#include <memory>
#include <unordered_map>

namespace Bot
{

class GroupList;
class Client;

}

namespace Utils
{

class BotConfig;

}

namespace Trigger
{

class ITrigger
{
public:
	ITrigger() = default;
	ITrigger(const ITrigger&) = delete;
	ITrigger& operator=(const ITrigger&) = delete;
	ITrigger(ITrigger&&) noexcept = default;
	ITrigger& operator=(ITrigger&&) noexcept = default;

	virtual void Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config) = 0;
	virtual time_t GetNext() = 0;

	virtual ~ITrigger() = default;
};

}

#endif