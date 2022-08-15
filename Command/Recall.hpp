#ifndef _RECALL_HPP_
#define _RECALL_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class Recall : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Recall";

	virtual int Permission(void) override { return 50; }
	virtual int Priority(void) override { return 20; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif