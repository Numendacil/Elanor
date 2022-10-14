#ifndef _RECALL_HPP_
#define _RECALL_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Recall : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PERMISSION = 20;
	static constexpr int GROUP_COMMAND_PRIORITY = 20;

public:
	static constexpr std::string_view _NAME_ = "Recall";

	int Permission() const override { return GROUP_COMMAND_PERMISSION; }
	int Priority() const override { return GROUP_COMMAND_PRIORITY; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

} // namespace GroupCommand

#endif