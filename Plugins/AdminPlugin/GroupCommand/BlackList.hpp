#ifndef _BLACK_LIST_HPP_
#define _BLACK_LIST_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class BlackList : public IGroupCommand
{
	static constexpr int GROUP_COMMAND_PERMISSION = 30;
public:
	static constexpr std::string_view _NAME_ = "BlackList";

	int Permission() const override { return GROUP_COMMAND_PERMISSION; }
	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	        	 Utils::BotConfig& config) override;
};

}

#endif