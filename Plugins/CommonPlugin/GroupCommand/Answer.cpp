#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/mirai.hpp>
#include <libmirai/Client.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

#include "Answer.hpp"
#include <Core/States/Activity.hpp>

using std::string;
using std::vector;

namespace GroupCommand
{

bool Answer::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client, Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	auto input = Utils::trim(str);
	if (input.size() > 1 && input[0] != '.') 
		return false;

	auto state = group.GetState<State::Activity>();
	if (!state->HasActivity())
		return false;
	if (state->GetActivityName() == "pjsk")
	{
		string answer{input.substr(1)};
		LOG_INFO(Utils::GetLogger(), "<Answer: pjsk>: " + answer + Utils::GetDescription(gm.GetSender()));
		state->AddAnswer({answer, gm.GetMessage().GetSourceInfo().value_or(Mirai::MessageChain::SourceInfo{}).id});
		return true;
	}
	return false;
}

}