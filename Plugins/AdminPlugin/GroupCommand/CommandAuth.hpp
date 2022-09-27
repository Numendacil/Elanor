#ifndef _COMMAND_AUTH_HPP_
#define _COMMAND_AUTH_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class CommandAuth : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PERMISSION = 50;
public:
	static constexpr std::string_view _NAME_ = "CommandAuth";

	int Permission() const override { return GROUP_COMMAND_PERMISSION; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	        	 Utils::BotConfig& config) override;
};

}

#endif