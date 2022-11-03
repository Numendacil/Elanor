#ifndef _PETPET_COMMAND_HPP_
#define _PETPET_COMMAND_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Petpet : public IGroupCommand
{
public:
	static constexpr std::string_view _NAME_ = "Petpet";

	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

}

#endif