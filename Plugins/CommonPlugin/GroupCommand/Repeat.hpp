#ifndef _REPEAT_HPP_
#define _REPEAT_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Repeat : public IGroupCommand
{
public:
	static constexpr std::string_view _NAME_ = "Repeat";

	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

}

#endif