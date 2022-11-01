#ifndef _IMAGE_SEARCH_COMMAND_HPP_
#define _IMAGE_SEARCH_COMMAND_HPP_

#include <Core/Interface/IGroupCommand.hpp>

namespace GroupCommand
{

class ImageSearch : public IGroupCommand
{
public:
	static constexpr std::string_view _NAME_ = "ImageSearch";

	bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
	             Utils::BotConfig& config) override;
};

}

#endif