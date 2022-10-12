#ifndef _PIXIV_COMMAND_HPP_
#define _PIXIV_COMMAND_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class PixivCommand : public IGroupCommand
{
public:
	static constexpr std::string_view _NAME_ = "Pixiv";

	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

} // namespace GroupCommand
#endif