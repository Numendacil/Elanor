#ifndef _ROLL_DICE_HPP_
#define _ROLL_DICE_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class RollDice : public IGroupCommand
{
public:
	static constexpr std::string_view _NAME_ = "RollDice";

	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client, Utils::BotConfig& config) override;
};

}

#endif