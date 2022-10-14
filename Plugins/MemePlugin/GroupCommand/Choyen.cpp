#include "Choyen.hpp"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/Client.hpp>
#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

#include <PluginUtils/Base64.hpp>

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool Choyen::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                     Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 2) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#choyen" && command != "#红字白字") return false;


	LOG_INFO(Utils::GetLogger(), "Calling Choyen <Choyen>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <Choyen>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}

	if (tokens.size() == 2)
	{
		string arg = Utils::toLower(tokens[1]);
		if (arg == "help" || arg == "h" || arg == "帮助")
		{
			LOG_INFO(Utils::GetLogger(), "帮助文档 <Choyen>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("usage:\n#choyen [line1] [line2]"));
		}
		else
		{
			LOG_INFO(Utils::GetLogger(), "缺少参数[line2] <Choyen>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("第二行字捏"));
		}
		return true;
	}

	std::string_view upper = tokens[1];
	std::string_view lower = tokens[2];
	if (upper.empty())
	{
		LOG_INFO(Utils::GetLogger(), "参数1为空 <Choyen>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("看不到第一句话捏，是口口剑22嘛"));
		return false;
	}
	if (lower.empty())
	{
		LOG_INFO(Utils::GetLogger(), "参数2为空 <Choyen>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("看不到第二句话捏，是口口剑22嘛"));
		return false;
	}
	bool UpperRainbow = (upper[0] == '$');
	if (UpperRainbow)
		upper.remove_prefix(1);
	bool LowerRainbow = (lower[0] == '$');
	if (LowerRainbow)
		lower.remove_prefix(1);

	LOG_INFO(Utils::GetLogger(), "生成 " 
		+ tokens[1] + " \\ " + tokens[2]
		+ " <Choyen>" + Utils::GetDescription(gm.GetSender(), false));

	std::string module = config.Get("/path/pymodules", "pymodules") + ".5000choyen";
	vector<string> cmd{
		"python", "-m", std::move(module),
		string(upper), string(lower) 
	};
	if (UpperRainbow)
		cmd.emplace_back("--rupper");
	if (LowerRainbow)
		cmd.emplace_back("--rlower");
	
	int status{};
	std::string output = Utils::exec(cmd, status);
	if (output.empty())
	{
		LOG_WARN(Utils::GetLogger(), "Error occured when executing 5000choyen <Choyen>");
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return true;
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		LOG_WARN(Utils::GetLogger(), 
			"Error occured when executing 5000choyen <Choyen>:\n"
			+ output
			+ "\nEXIT STATUS: " + std::to_string(WEXITSTATUS(status))
		);
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return true;
	}
	
	LOG_INFO(Utils::GetLogger(), "上传图片 <Choyen>" + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(group.gid, Mirai::MessageChain().Image("", "", "", Utils::b64encode(output)));
	return true;
}

}