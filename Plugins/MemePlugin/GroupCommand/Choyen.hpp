#ifndef _CHOYEN_COMMAND_HPP_
#define _CHOYEN_COMMAND_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Choyen : public IGroupCommand
{
public:
	static constexpr std::string_view _NAME_ = "Choyen";

	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

}

#endif