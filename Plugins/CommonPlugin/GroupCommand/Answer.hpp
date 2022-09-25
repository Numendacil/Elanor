#ifndef _ANSWER_HPP_
#define _ANSWER_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Answer : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PERMISSION = 0;
	static constexpr int GROUP_COMMAND_PRIORITY = 5;
public:
	static constexpr std::string_view _NAME_ = "Answer";

	int Permission() const override { return GROUP_COMMAND_PERMISSION; }
	int Priority() const override { return GROUP_COMMAND_PRIORITY; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	        	Utils::BotConfig& config) override;
};

}

#endif