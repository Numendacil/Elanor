#ifndef _CHOYEN_HPP_
#define _CHOYEN_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class Choyen: public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Choyen";

	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif