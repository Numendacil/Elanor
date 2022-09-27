#ifndef _REPEAT_HPP_
#define _REPEAT_HPP_

#include <Command/GroupCommandBase.hpp>


namespace GroupCommand
{

class Repeat : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Repeat";

	virtual int Priority(void) override { return 0; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif