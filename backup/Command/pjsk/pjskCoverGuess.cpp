#include <fstream>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <State/Activity.hpp>
#include <ThirdParty/log.h>
#include <ThirdParty/uuid.h>
#include <nlohmann/json.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <Utils/Utils.hpp>

#include <libmirai/mirai.hpp>

#include "pjskCoverGuess.hpp"

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool pjskCoverGuess::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() >= std::char_traits<char>::length("#pjsk 猜曲绘"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(tokens, str) < 2)
			return false;
		if (tokens[0] == "#pjsk" || tokens[0] == "#啤酒烧烤" || tokens[0] == "#prsk")
			if (tokens[1] == "猜曲绘")
				return true;
	}
	return false;
}



bool pjskCoverGuess::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 1);
	logging::INFO("Calling pjskCoverGuess <pjskCoverGuess>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();

	auto state = group.GetState<State::Activity>();
	auto holder = state->CheckAndStart("pjsk");
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
	if (!holder)
	{
		logging::INFO("有活动正在进行 <pjskCoverGuess>" + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("有活动正在进行中捏"));
		return false;
	}

	json music, alias;
	{
		std::ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");

		if (!ifile)
		{
			logging::WARN("Unable to open meta.json <pjskCoverGuess>");
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}

		json meta_data = json::parse(ifile);
		ifile.close();

		std::uniform_int_distribution<int> rng1(0, meta_data.size() - 1);
		do
		{
			music = meta_data[rng1(Utils::rng_engine)];
		}
		while (!std::filesystem::exists(MediaFilesPath + "images/pjsk/cover/" + music["assetbundleName"].get<string>() + ".png"));
	}
	{
		std::ifstream ifile(MediaFilesPath + "music/pjsk/alias.json");

		if (!ifile)
		{
			logging::WARN("Unable to open alias.json <pjskCoverGuess>");
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}

		json alias_data = json::parse(ifile);
		ifile.close();

		for (const auto& p : alias_data.items())
		{
			if (p.value()["musicId"] == music["musicId"])
			{
				alias = p.value()["alias"];
				break;
			}
		}
	}
	
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("请在规定时间内发送曲绘对应的歌曲名称。回答请以句号开头捏"));
	logging::INFO("题目为 <pjskCoverGuess>: " + music.dump() + alias.dump());

	string cover, cover_ans_path;
	int width, height;
	{
		cover = music["assetbundleName"].get<string>();
		if (music["hasOriginCover"].get<bool>())
		{
			std::bernoulli_distribution rng2(0.5);
			cover += (rng2(Utils::rng_engine)) ? "" : "_org";
		}
		cover_ans_path = MediaFilesPath + "images/pjsk/cover_small/" + cover + "_small.png";
		cover = MediaFilesPath + "images/pjsk/cover/" + cover + ".png";
		const string shape = Utils::exec({"identify",
						  "-format", "%[fx:w] %[fx:h]",
						  cover});
		vector<string> s;
		if (Utils::Tokenize(s, shape) < 2)
		{
			logging::WARN("Failed to get image size <pjskCoverGuess>: " + shape);
			width = 780;
			height = 780;
		}
		else
		{
			width = stoi(s[0]);
			height = stoi(s[1]);
		}
	}

	std::unordered_set<string> alias_map;
	for (const auto &item : alias.items())
	{
		string str = string(item.value());
		Utils::ToLower(str);
		Utils::ReplaceMark(str);
		alias_map.insert(str);
	}

	const int round = Utils::Configs.Get<int>("/pjsk/CoverGuess/Round"_json_pointer, 2);
	const double ratio = Utils::Configs.Get<double>("/pjsk/CoverGuess/Ratio"_json_pointer, 0.15);
	const double incr = Utils::Configs.Get<double>("/pjsk/CoverGuess/IncrRate"_json_pointer, 0.8);
	int x, y;
	{
		int width_target = (int)floor(width * ratio * ((round - 1) * 0.5 + 1.0));
		int height_target = (int)floor(height * ratio * ((round - 1) * 0.5 + 1.0));
		std::uniform_int_distribution<int> rng_x(5 + width_target / 2, width - 5 - width_target / 2);
		std::uniform_int_distribution<int> rng_y(5 + width_target / 2, height - 5 - height_target / 2);
		x = rng_x(Utils::rng_engine);
		y = rng_y(Utils::rng_engine);
	}
	for (int i = 0 ; i < round; ++i)
	{
		string cover_path;
		{
			uuids::basic_uuid_random_generator rng(Utils::rng_engine);
			cover_path = MediaFilesPath + "tmp/" + uuids::to_string(rng()) + ".png";

			
			int width_target = (int)floor(width * ratio * (i * incr + 1.0));
			int height_target = (int)floor(height * ratio * (i * incr + 1.0));

			Utils::exec({"convert",
				     "-crop", std::to_string(width_target) + "x" + std::to_string(height_target) + "+" + std::to_string(int(x - width_target / 2)) + "+" + std::to_string(int(y - width_target / 2)),
				     cover,
				     cover_path});
		}
		logging::INFO("图片处理完毕 <pjskCoverGuess>: " + cover_path);
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Image("", "", cover_path, ""));

		const auto tp = std::chrono::system_clock::now();
		bool flag = false;
		while (true)
		{
			State::Activity::AnswerInfo info;
			if (!state->WaitForAnswerUntil(tp + std::chrono::seconds(21), info))
				break;
			Utils::ToLower(info.answer);
			Utils::ReplaceMark(info.answer);
			if (alias_map.contains(info.answer))
			{
				logging::INFO("回答正确 <pjskCoverGuess>: " + info.answer + "\t-> [" + gm.GetSender().group.name + "(" + gm.GetSender().group.id.to_string() + ")]");
				client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("回答正确"), info.message_id);
				flag = true;
				break;
			}
		}
		if (flag) break;
	}
	
	string answer = music["title"].get<string>();

	logging::INFO("公布答案 <pjskCoverGuess>: " + answer
					+ "\t-> [" + gm.GetSender().group.name + "(" + gm.GetSender().group.id.to_string() + ")]");
	client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain()
				.Plain("正确答案是:   " + answer + "\n")
				.Image("", "", cover_ans_path, ""));

	return true;
}

}