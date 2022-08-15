#include <ThirdParty/log.h>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <State/CoolDown.hpp>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include <libmirai/mirai.hpp>

#include "Pixiv.hpp"

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool Pixiv::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() > std::char_traits<char>::length("#pix"))
	{
		if (Utils::Tokenize(tokens, str) < 2)
			return false;
		Utils::ToLower(tokens[0]);
		if (tokens[0] == "#pixiv" || tokens[0] == "#p站" || tokens[0] == "#pix")
			return true;
	}
	return false;
}

bool GetPixivById(const Mirai::GroupMessageEvent& gm, Bot::Group& group, long pid, int page)
{
	using namespace date;
	using namespace std::chrono;
	Bot::Client& client = Bot::Client::GetClient();
	auto cd = group.GetState<State::CoolDown>();
	std::chrono::seconds remaining;
	auto holder = cd->GetRemaining("Pixiv", 20s, remaining);

	if (remaining > 0s)
	{
		std::stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余<Pixiv>: " + ss.str() + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);
	const string url_local = Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000");
	httplib::Client cli(url_local);
	Utils::SetClientOptions(cli);
	auto result = cli.Get("/pixiv/id/", {{"id", std::to_string(pid)}, {"page", std::to_string(page)}}, {{"Accept-Encoding", "gzip"}});
	if (!Utils::CheckHttpResponse(result, "Pixiv"))
	{
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("info"));
	if (!msg.contains("image"))
	{
		logging::INFO("上传结果<Pixiv>: 未找到图片" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(msg["info"].get<string>()));
		return true;
	}
	json info = json::parse(msg["info"].get<string>());
	string message;
	try
	{
		message = "标题: " + info["title"].get<string>() + " (id: " + to_string(info["id"]) + ")\n作者: " + info["user"]["name"].get<string>() + " (id: " + to_string(info["user"]["id"]) + ")\n标签:";
		for (auto p = info["tags"].begin(); p < info["tags"].end() && p < info["tags"].begin() + 7; p++)
		{
			if (!(*p)["translated_name"].empty() && (*p)["name"].get<string>().find("users入り") == string::npos)
			{
				message += " #" + (*p)["name"].get<string>() + " (" + (*p)["translated_name"].get<string>() + ")  ";
			}
			else
			{
				message += " #" + (*p)["name"].get<string>() + "  ";
			}
		}
		if (info["tags"].size() > 7)
		{
			message += " ...";
		}
		message += '\n';
		int num_page = info["page_count"].get<int>();
		if (num_page > 1)
		{
			message += "页码: " + std::to_string((num_page > page - 1) ? page : 1) + "/" + std::to_string(num_page) + "\n";
		}
	}
	catch (const std::exception& e)
	{
		logging::WARN("Error occured while parsing result <Pixiv>: " +string(e.what()) + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}

	logging::INFO("上传结果 <Pixiv>" + Utils::GetDescription(gm, false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(message).Image("", "", "", msg["image"]));
	return true;
}

bool Pixiv::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	logging::INFO("Calling Pixiv<Pixiv>" + Utils::GetDescription(gm));
	assert(tokens.size() > 1);
	Bot::Client& client = Bot::Client::GetClient();
	string command = tokens[1];
	Utils::ToLower(command);
	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档<Pixiv>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("usage:\n#pixiv id [pid] (page)"));
		return true;
	}
	else if (command == "id")
	{
		if (tokens.size() < 3)
		{
			logging::INFO("缺少参数[pid] <Pixiv>" + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("不发pid看个锤子图"));
			return true;
		}
		long pid = 0;
		int page = 1;
		try
		{
			pid = stol(tokens[2]);
			if (pid <= 0)
				throw std::logic_error("[pid] must be positive");
		}
		catch (const std::logic_error& e)
		{
			logging::INFO("无效[pid] <Pixiv>: " + tokens[2] + Utils::GetDescription(gm, false));
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[2] + "是个锤子pid"));
			return false;
		}
		if (tokens.size() > 3)
		{
			try
			{
				page = stoi(tokens[3]);
				if (page <= 0)
					throw std::logic_error("[page] must be positive");
			}
			catch (const std::logic_error &e)
			{
				logging::INFO("无效[page] <Pixiv>: " + tokens[3] + Utils::GetDescription(gm, false));
				client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[3] + "是个锤子页码"));
				return false;
			}
		}
		return GetPixivById(gm, group, pid, page);
	}

	logging::INFO("未知指令 <Pixiv>: " + tokens[1] + Utils::GetDescription(gm, false));
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(tokens[1] + "是什么指令捏，不认识捏"));
	return false;
}

}