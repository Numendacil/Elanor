#ifndef _BILILIVE_HPP_
#define _BILILIVE_HPP_

#include <Command/GroupCommandBase.hpp>


namespace GroupCommand
{

class Bililive: public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Bililive";

	virtual int Permission(void) override{ return 10; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif