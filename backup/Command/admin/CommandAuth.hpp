#ifndef _COMMAND_AUTH_HPP_
#define _COMMAND_AUTH_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class CommandAuth : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "CommandAuth";

	virtual int Permission(void) override { return 50; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif