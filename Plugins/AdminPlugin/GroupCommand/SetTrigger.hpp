#ifndef _SET_TRIGGER_HPP_
#define _SET_TRIGGER_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class SetTrigger : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PERMISSION = 30;
public:
	static constexpr std::string_view _NAME_ = "SetTrigger";

	int Permission() const override { return GROUP_COMMAND_PERMISSION; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	        	 Utils::BotConfig& config) override;
};

}

#endif