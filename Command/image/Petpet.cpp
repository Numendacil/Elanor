#include <ThirdParty/log.h>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include <libmirai/mirai.hpp>

#include "Petpet.hpp"

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool Petpet::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() >= std::char_traits<char>::length("#pet"))
	{
		Utils::Tokenize(tokens, str);
		Utils::ToLower(tokens[0]);
		if (tokens[0] == "#pet" || tokens[0] == "#摸摸")
			return true;
	}
	return false;
}

bool Petpet::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	logging::INFO("Calling Petpet <Petpet>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	Mirai::QQ_t target;
	if (tokens.size() > 1)
	{
		if (tokens[1].empty())
		{
			logging::INFO("参数为空 <Petpet>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("QQ号看不见捏，怎么会事捏"));
			return false;
		}
		if (tokens[1] == "help" || tokens[1] == "h" || tokens[1] == "帮助")
		{
			logging::INFO("帮助文档 <Petpet>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#pet [QQ]"));
			return true;
		}
		else
		{
			try
			{
				target = (Mirai::QQ_t)stol(tokens[1]);
			}
			catch (const std::logic_error& e)
			{
				logging::INFO("无效参数 <Petpet>: " + tokens[1] + Utils::GetDescription(gm, false));
				client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[1] + "是个锤子QQ号"));
				return false;
			}
		}
	}

	auto AtMsg = gm.GetMessage().GetAll<Mirai::AtMessage>();
	if (!AtMsg.empty())
		target = AtMsg[0].GetTarget();

	if (target != Mirai::QQ_t{})
	{
		if (target == gm.GetMiraiClient().GetBotQQ())
		{
			std::uniform_int_distribution<int> rngroll(0, 4);
			if (!rngroll(Utils::rng_engine))
			{
				logging::INFO("你吗 <Petpet>" + Utils::GetDescription(gm, false));
				client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("摸你吗个头，爬"));
				return true;
			}
		}

		httplib::Client cli(Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000"));
		Utils::SetClientOptions(cli);
		auto result = cli.Get("/gen/pet/", {{"qq", target.to_string()}}, {{"Accept-Encoding", "gzip"}});
		if (!Utils::CheckHttpResponse(result, "Petpet"))
		{
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}
		json msg = json::parse(result->body);
		
		assert(msg.contains("result"));
		logging::INFO("上传gif <Petpet>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Image("", "", "", msg["result"]));
		return true;
	}
	else
	{
		logging::INFO("缺少target <Petpet>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("你搁这摸空气呢"));
		return false;
	}
}

}