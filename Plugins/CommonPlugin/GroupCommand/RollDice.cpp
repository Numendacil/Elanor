#include "RollDice.hpp"

#include <array>
#include <charconv>
#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/Messages/MessageChain.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

using std::string;
using std::string_view;
using std::vector;

namespace GroupCommand
{

bool RollDice::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                       Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') 
		return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 1) return false;
	if (Utils::toLower(tokens[0]) != "#roll") return false;


	LOG_INFO(Utils::GetLogger(), "Calling RollDice <RollDice>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <RollDice>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}

	int round = 1;
	int bound = 100; // NOLINT(*-avoid-magic-numbers)

	constexpr size_t MAX_ROUND = 10;
	std::array<int, MAX_ROUND> result{};

	if (tokens.size() >= 2)
	{
		string command = Utils::toLower(tokens[1]);
		if (command == "help" || command == "h")
		{
			LOG_INFO(Utils::GetLogger(), "帮助文档 <RollDice>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("usage:\n#roll [x]D[y]"));
			return true;
		}

		auto delim = command.find('d');
		string_view round_str, bound_str;
		if (delim == string::npos)
		{
			round_str = command;
		}
		else
		{
			round_str = command;
			round_str = round_str.substr(0, delim);

			bound_str = command;
			bound_str = bound_str.substr(delim + 1);
		}

		if (!round_str.empty())
		{
			auto result = std::from_chars(round_str.data(), round_str.data() + round_str.size(), round);
			if (result.ec == std::errc::invalid_argument)
			{
				LOG_INFO(Utils::GetLogger(),
				         "格式错误 <RollDice>: round = " + string(round_str)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid,
				                        Mirai::MessageChain().Plain(string(round_str) + "是什么数字捏，不认识捏"));
				return true;
			}
			if (result.ec == std::errc::result_out_of_range)
			{
				LOG_INFO(Utils::GetLogger(),
				         "数字溢出 <RollDice>: round = " + string(round_str)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("数字太、太大了"));
				return true;
			}
			if (round < 1)
			{
				LOG_INFO(Utils::GetLogger(),
				         "投掷次数错误 <RollDice>: round = " + std::to_string(round)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("骰子不见了捏，怎么会事捏"));
				return true;
			}
			if (round > MAX_ROUND)
			{
				LOG_INFO(Utils::GetLogger(),
				         "投掷次数错误 <RollDice>: round = " + std::to_string(round)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("骰子太多啦！"));
				return true;
			}
		}

		if (!bound_str.empty())
		{
			auto result = std::from_chars(bound_str.data(), bound_str.data() + bound_str.size(), bound);
			if (result.ec == std::errc::invalid_argument)
			{
				LOG_INFO(Utils::GetLogger(),
				         "格式错误 <RollDice>: bound = " + string(bound_str)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid,
				                        Mirai::MessageChain().Plain(string(bound_str) + "是什么数字捏，不认识捏"));
				return true;
			}
			if (result.ec == std::errc::result_out_of_range)
			{
				LOG_INFO(Utils::GetLogger(),
				         "数字溢出 <RollDice>: bound = " + string(bound_str)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("数字太、太大了"));
				return true;
			}
			if (bound < 1)
			{
				LOG_INFO(Utils::GetLogger(),
				         "骰子面数过小 <RollDice>: bound = " + std::to_string(bound)
				             + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("这是什么奇妙骰子捏，没见过捏"));
				return true;
			}
		}
	}

	std::uniform_int_distribution<int> rngroll(1, bound);
	int ans = 0;
	string msg;
	for (int i = 0; i < round; ++i)
	{
		result[i] = rngroll(Utils::GetRngEngine()); // NOLINT(*-bounds-constant-array-index)
		ans += result[i];                           // NOLINT(*-bounds-constant-array-index)
		msg += (i) ? " + " + std::to_string(result[i])	// NOLINT(*-bounds-constant-array-index)
				   : std::to_string(result[i]); // NOLINT(*-bounds-constant-array-index)
	}
	msg += " = ";
	LOG_INFO(Utils::GetLogger(),
	         "随机数生成 <RollDice>: " + msg + std::to_string(ans) + Utils::GetDescription(gm.GetSender(), false));
	if (round == 1)
		client.SendGroupMessage(
			group.gid, Mirai::MessageChain().Plain(gm.GetSender().MemberName + " 掷出了: " + std::to_string(ans)));
	else
		client.SendGroupMessage(
			group.gid,
			Mirai::MessageChain().Plain(gm.GetSender().MemberName + " 掷出了: " + msg + std::to_string(ans)));
	return true;
}

} // namespace GroupCommand