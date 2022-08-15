#ifndef _SET_TRIGGER_HPP_
#define _SET_TRIGGER_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class SetTrigger : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "SetTrigger";

	virtual int Permission(void) override { return 50; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif