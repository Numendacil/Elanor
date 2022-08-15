#ifndef _PJSK_CHART_HPP_
#define _PJSK_CHART_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class pjskChart : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "pjskChart";

	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif