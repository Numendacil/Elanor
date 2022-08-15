#ifndef _SING_HPP_
#define _SING_HPP_

#include <Command/GroupCommandBase.hpp>


namespace GroupCommand
{

class Sing : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Sing";

	virtual int Priority(void) override { return 10; }
	virtual int Permission(void) override { return 100; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif