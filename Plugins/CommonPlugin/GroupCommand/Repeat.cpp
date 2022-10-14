#include "Repeat.hpp"

#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/Messages/MessageChain.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/States/CustomState.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

using std::string;
using std::vector;
using json = nlohmann::json;

namespace GroupCommand
{

struct RepeatState
{
	Mirai::MessageChain LastMsg{};
	bool isRepeated = false;
};

void from_json(const json& j, RepeatState& p)
{
	p.LastMsg = j.value("LastMsg", Mirai::MessageChain{});
	p.isRepeated = j.value("isRepeated", false);
}

void to_json(json& j, const RepeatState& p)
{
	j["LastMsg"] = p.LastMsg;
	j["isRepeated"] = p.isRepeated;
}

bool Repeat::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                     Utils::BotConfig& config)
{
	auto msg = gm.GetMessage();
	string str = msg.ToJson().dump();
	if (str.empty() || str == "[]") return true;

	group.GetState<State::CustomState>()->ModifyState(
		"RepeatState",
		[&](json& state)
		{
			RepeatState repeat_state = state.get<RepeatState>();

			if (str == repeat_state.LastMsg.ToJson().dump())
			{
				LOG_DEBUG(Utils::GetLogger(), "有人复读 <Repeat>: " + str + Utils::GetDescription(gm.GetSender()));
				if (!repeat_state.isRepeated)
				{
					constexpr int REPEAT_PROB = 10;
					std::uniform_int_distribution rng_repeat(1, REPEAT_PROB);
					if (rng_repeat(Utils::GetRngEngine()) == 1)
					{
						repeat_state.LastMsg = msg;
						repeat_state.isRepeated = true;
						client.SendGroupMessage(group.gid, msg);
						LOG_INFO(Utils::GetLogger(),
					             "bot复读成功 <Repeat>" + Utils::GetDescription(gm.GetSender(), false));
					}
				}
			}
			else
			{
				repeat_state.LastMsg = msg;
				repeat_state.isRepeated = false;
			}

			state = repeat_state;
		},
		RepeatState{});
	return true;
}

} // namespace GroupCommand