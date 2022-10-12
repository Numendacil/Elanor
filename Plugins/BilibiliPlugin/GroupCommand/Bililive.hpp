#ifndef _BILILIVE_HPP_
#define _BILILIVE_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class Bililive : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PERMISSION = 20;

public:
	static constexpr std::string_view _NAME_ = "Bililive";

	int Permission() const override { return GROUP_COMMAND_PERMISSION; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

} // namespace GroupCommand

#endif