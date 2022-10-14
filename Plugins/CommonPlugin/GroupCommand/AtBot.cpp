#include "AtBot.hpp"

#include <array>
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

using std::string;
using std::vector;

namespace GroupCommand
{

bool AtBot::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                    Utils::BotConfig& config)
{
	auto AtList = gm.GetMessage().GetAll<Mirai::AtMessage>();
	if (AtList.empty()) return false;

	bool at = false;
	for (const auto& p : AtList)
	{
		if (p.GetTarget() == client->GetBotQQ())
		{
			at = true;
			break;
		}
	}
	if (!at) return false;


	LOG_INFO(Utils::GetLogger(), "有人@bot <AtBot>" + Utils::GetDescription(gm.GetSender()));
	constexpr std::array words = {"干嘛", "？"};
	constexpr size_t words_count = words.size();

	const std::filesystem::path filepath = config.Get("/path/MediaFiles", std::filesystem::path("media_files")) / std::filesystem::path("images/at");
	vector<string> image;
	for (const auto& entry : std::filesystem::directory_iterator(filepath))
	{
		if (std::filesystem::is_regular_file(entry)) image.push_back(entry.path());
	}

	size_t idx = std::uniform_int_distribution<size_t>(0, words_count + image.size() - 1)(Utils::GetRngEngine());
	if (idx >= words_count)
	{
		LOG_INFO(Utils::GetLogger(),
		         "回复 <AtBot>: " + image[idx - words_count] + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(gm.GetSender().group.id,
		                        Mirai::MessageChain().Image("", "", image[idx - words_count], ""));
	}
	else
	{
		LOG_INFO(Utils::GetLogger(),
		         "回复 <AtBot>: " + string(words[idx]) // NOLINT(*-bounds-constant-array-index)
		             + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(gm.GetSender().group.id,
		                        Mirai::MessageChain().Plain(words[idx])); // NOLINT(*-bounds-constant-array-index)
	}
	return true;
}

} // namespace GroupCommand