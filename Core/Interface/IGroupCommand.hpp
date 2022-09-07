#ifndef _ELANOR_CORE_GROUP_COMMAND_INTERFACE_HPP_
#define _ELANOR_CORE_GROUP_COMMAND_INTERFACE_HPP_

#include <any>
#include <string>

#include <libmirai/Events/GroupMessageEvent.hpp>
#include <libmirai/Messages/MessageChain.hpp>

namespace Bot
{

class Client;
class Group;

}

namespace GroupCommand
{

static constexpr int GROUP_COMMAND_DEFAULT_PRIORITY = 10;
static constexpr int GROUP_COMMAND_DEFAULT_PERMISSION = 0;

class IGroupCommand
{
public:
	virtual int Permission() const { return GROUP_COMMAND_DEFAULT_PERMISSION; }
	virtual int Priority() const { return GROUP_COMMAND_DEFAULT_PRIORITY; }
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client) = 0;

	virtual ~IGroupCommand() = default;
};

} // namespace GroupCommand

#endif