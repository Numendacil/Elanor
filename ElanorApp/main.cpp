#include <exception>
#include <string>
#include <iostream>
#include <libmirai/mirai.hpp>

#include "Core/Utils/Logger.hpp"
#include "ElanorBot.hpp"
#include "libmirai/Utils/Logger.hpp"

using std::string;
int main()
{
	Bot::ElanorBot Bot;
	Bot.SetConfig("./bot_config.json");
	Mirai::SessionConfigs opts;
	try
	{
		opts.FromFile("./client_config.json");
	}
	catch (const std::exception& e)
	{
		LOG_ERROR(Utils::GetLogger(), e.what());
		return 0;
	}
	
	Bot.Start(opts);

	string cmd;
	while (std::cin >> cmd)
	{
		if (cmd == "exit")
		{
			Bot.Stop();
			break;
		}
	}
	return 0;

}