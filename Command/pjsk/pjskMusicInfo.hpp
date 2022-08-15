#ifndef _PJSK_MUSIC_INFO_HPP_
#define _PJSK_MUSIC_INFO_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class pjskMusicInfo : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "pjskMusicInfo";

	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif