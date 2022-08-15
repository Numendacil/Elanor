#ifndef _PJSK_SONG_GUESS_HPP_
#define _PJSK_SONG_GUESS_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class pjskSongGuess : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "pjskSongGuess";

	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif