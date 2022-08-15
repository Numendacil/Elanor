#include <ThirdParty/log.h>
#include <State/Activity.hpp>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include "Answer.hpp"


namespace GroupCommand
{

bool Answer::Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens)
{
	std::string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str[0] != '.')
		return false;
	tokens.clear();
	tokens.push_back(str.substr(1));
	return true;
}

bool Answer::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) 
{
	auto state = group.GetState<State::Activity>();
	if (!state->HasActivity())
		return false;
	if (state->GetActivityName() == "pjsk")
	{
		assert(!tokens.empty());
		logging::INFO("<Answer: pjsk>: " + tokens[0] + Utils::GetDescription(gm));
		state->AddAnswer({tokens[0], gm.GetMessage().GetSourceInfo()->id});
		return true;
	}
	return false;
}

}