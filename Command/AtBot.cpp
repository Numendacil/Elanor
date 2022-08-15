#include <filesystem>
#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include "AtBot.hpp"

using std::string;
using std::vector;

namespace GroupCommand
{

bool AtBot::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	if (msg.GetAll<Mirai::AtMessage>().empty())
		return false;
	return true;
}

bool AtBot::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	bool at = false;
	for (const auto& p : gm.GetMessage().GetAll<Mirai::AtMessage>())
	{
		if (p.GetTarget() == gm.GetMiraiClient().GetBotQQ())
		{
			at = true;
			break;
		}
	}
	if (!at)
		return false;

	Bot::Client& client = Bot::Client::GetClient();

	logging::INFO("有人@bot <AtBot>" + Utils::GetDescription(gm));
	const vector<string> words = {"干嘛", "？"};
	const int words_count = words.size();

	const std::filesystem::path filepath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/") + "images/at";
	vector<string> image;
	for (const auto & entry : std::filesystem::directory_iterator(filepath))
	{
		if (std::filesystem::is_regular_file(entry))
			image.push_back(entry.path());
	}

	int idx = std::uniform_int_distribution<int>(0, words_count + image.size() - 1)(Utils::rng_engine);
	if (idx >= words_count)
	{
		logging::INFO("回复 <AtBot>: " + image[idx - words_count] + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Image("", "", image[idx - words_count], ""));
	}
	else
	{
		logging::INFO("回复 <AtBot>: " + words[idx] + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(words[idx]));
	}
	return true;
}

}