#include <chrono>
#include <Utils/Utils.hpp>
#include <Client/Client.hpp>

#include <libmirai/mirai.hpp>


#include "ElanorBot.hpp"

using std::string;
using namespace std::chrono_literals;

int main()
{
	Utils::Configs.FromFile("./bot_config.json");
	Mirai::SessionConfigs opts;
	opts.FromFile("./client_config.json");
	const Mirai::QQ_t Owner = (Mirai::QQ_t)Utils::Configs.Get<int64_t>("/OwnerQQ"_json_pointer, 12345);

	Bot::ElanorBot Bot(Owner);
	Mirai::MiraiClient& client = Bot::Client::GetClient().GetMiraiClient();

	client.On<Mirai::NudgeEvent>([&Bot](Mirai::NudgeEvent e)
	{
		Bot.NudgeEventHandler(e);
	});

	client.On<Mirai::GroupMessageEvent>([&Bot](Mirai::GroupMessageEvent e)
	{
		Bot.GroupMessageEventHandler(e);
	});

	client.On<Mirai::ClientConnectionEstablishedEvent>([&Bot](Mirai::ClientConnectionEstablishedEvent e)
	{
		Bot.ConnectionOpenedHandler(e);
	});

	client.On<Mirai::ClientConnectionClosedEvent>([&Bot](Mirai::ClientConnectionClosedEvent e)
	{
		Bot.ConnectionClosedHandler(e);
	});

	client.On<Mirai::ClientConnectionErrorEvent>([&Bot](Mirai::ClientConnectionErrorEvent e)
	{
		Bot.ConnectionErrorHandler(e);
	});

	client.On<Mirai::ClientParseErrorEvent>([&Bot](Mirai::ClientParseErrorEvent e)
	{
		Bot.ParseErrorHandler(e);
	});

	while (true)
	{
		try
		{
			logging::INFO("尝试与 mirai-api-http 建立连接...");
			Bot::Client::GetClient().Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			logging::WARN(ex.what());
		}
		std::this_thread::sleep_for(3s);
	}

	// 检查一下版本
	try
	{
		string mah_version = client.GetMiraiApiHttpVersion();
		string mc_version = string(client.GetVersion());
		logging::INFO("mirai-api-http 的版本: " + mah_version
			+ "; mirai-cpp 的版本: " + mc_version);
		if (mah_version != mc_version)
		{
			logging::WARN("Warning: 你的 mirai-api-http 插件的版本与 mirai-cpp 的版本不同，可能存在兼容性问题。");
		}
	}
	catch (const std::exception& ex)
	{
		logging::WARN(ex.what());
	}
	logging::INFO("Bot Working...");



	string cmd;
	while (std::cin >> cmd)
	{
		if (cmd == "exit")
		{
			Bot::Client::GetClient().Disconnect();
			break;
		}
	}
	return 0;

}