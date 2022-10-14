#ifndef _REPEAT_HPP_
#define _REPEAT_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Repeat : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PRIORITY = 0;
public:
	static constexpr std::string_view _NAME_ = "Repeat";

	int Priority() const override { return GROUP_COMMAND_PRIORITY; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

} // namespace GroupCommand

#endif