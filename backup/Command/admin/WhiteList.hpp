#ifndef _WHITE_LIST_HPP_
#define _WHITE_LIST_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class WhiteList : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "WhiteList";

	virtual int Permission(void) override { return 100; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif