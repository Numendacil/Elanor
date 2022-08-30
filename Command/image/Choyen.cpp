#include <ThirdParty/log.h>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include "Choyen.hpp"

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool Choyen::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() > std::char_traits<char>::length("#choyen"))
	{
		if (Utils::Tokenize(tokens, str, 3) < 2)
			return false;
		Utils::ToLower(tokens[0]);
		if (tokens[0] == "#choyen" || tokens[0] == "#红字白字")
			return true;
	}
	return false;
}

bool Choyen::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	logging::INFO("Calling Choyen <Choyen>" + Utils::GetDescription(gm));
	assert(tokens.size() > 1);
	Bot::Client& client = Bot::Client::GetClient();

	if (tokens.size() == 2)
	{
		if (tokens[1] == "help" || tokens[1] == "h" || tokens[1] == "帮助")
		{
			logging::INFO("帮助文档 <Choyen>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#choyen [line1] [line2]"));
			return true;
		}
		else
		{
			logging::INFO("缺少参数[line2] <Choyen>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("你的第二行字捏"));
			return false;
		}
	}
	
	assert(tokens.size() > 2);
	std::string_view upper = tokens[1];
	std::string_view lower = tokens[2];
	if (upper.empty())
	{
		logging::INFO("参数1为空 <Choyen>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("看不到第一句话捏，是口口剑22嘛"));
		return false;
	}
	if (lower.empty())
	{
		logging::INFO("参数2为空 <Choyen>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("看不到第二句话捏，是口口剑22嘛"));
		return false;
	}

	bool UpperRainbow = (upper[0] == '$');
	if (UpperRainbow)
		upper.remove_prefix(1);
	bool LowerRainbow = (lower[0] == '$');
	if (LowerRainbow)
		lower.remove_prefix(1);

	const string url_local = Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000");
	httplib::Client cli(url_local);
	Utils::SetClientOptions(cli);
	auto result = cli.Get("/gen/choyen/", {{"upper", string(upper)}, {"lower", string(lower)}, {"rainbow_upper", UpperRainbow ? "true" : "false"}, {"rainbow_lower", LowerRainbow ? "true" : "false"}}, {{"Accept-Encoding", "gzip"}});
	if (!Utils::CheckHttpResponse(result, "Choyen"))
	{
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("result"));
	logging::INFO("上传图片 <Choyen>" + Utils::GetDescription(gm, false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Image("", "", "", msg["result"]));
	return true;
}

}