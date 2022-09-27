#ifndef _BLACK_LIST_HPP_
#define _BLACK_LIST_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class BlackList : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "BlackList";

	virtual int Permission(void) override{ return 50; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif