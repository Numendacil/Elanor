#ifndef _AT_HPP_
#define _AT_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class AtBot : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "At";

	virtual int Priority(void) override { return 1; }
	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}
#endif