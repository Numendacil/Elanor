#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/LastMessage.hpp>

#include "Repeat.hpp"

using std::string;
using std::vector;

namespace GroupCommand
{

bool Repeat::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.ToJson().dump();
	if (str.empty() || str == "[]")
		return false;
	tokens.clear();
	tokens.push_back(str);
	return true;
}

bool Repeat::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	Mirai::MessageChain LastMsg;
	bool Repeated;
	auto state = group.GetState<State::LastMessage>();
	state->Get(LastMsg, Repeated);

	if (tokens[0] == LastMsg.ToJson().dump())
	{
	//	logging::INFO("有人复读 <Repeat>: " + tokens[0] + Utils::GetDescription(gm));
		if (!Repeated)
		{
			std::uniform_int_distribution rng_repeat(1, 10);
			if (rng_repeat(Utils::rng_engine) == 1)
			{
				state->Set(gm.GetMessage(), true);
				Bot::Client::GetClient().SendGroupMessage(gm.GetSender().group.id, gm.GetMessage());
				logging::INFO("bot复读成功 <Repeat>" + Utils::GetDescription(gm, false));
				return true;
			}
		}
	}
	else
	{
		state->Set(gm.GetMessage(), false);
	}
	return false;
}

}