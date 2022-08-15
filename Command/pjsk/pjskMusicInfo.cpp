#include <filesystem>
#include <nlohmann/json.hpp>
#include <ThirdParty/date.h>
#include <State/CoolDown.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <Utils/Utils.hpp>

#include "pjskMusicInfo.hpp"

using namespace date;
using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

bool pjskMusicInfo::Parse(const Mirai::MessageChain& msg, vector<string>& tokens)
{
	string str = Utils::GetText(msg);
	Utils::ReplaceMark(str);
	if (str.length() >= std::char_traits<char>::length("#pjsk info"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(tokens, str) < 3)
			return false;
		if (tokens[0] == "#pjsk" || tokens[0] == "#啤酒烧烤" || tokens[0] == "#prsk")
			if (tokens[1] == "info" || tokens[1] == "歌曲信息")
			return true;
	}
	return false;
}



bool pjskMusicInfo::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 2);
	logging::INFO("Calling pjskMusicInfo <pjskMusicInfo>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	string target = tokens[2];
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");

	json music;
	{
		vector<std::pair<json, std::unordered_set<string>>> alias_pair;
		std::ifstream ifile(MediaFilesPath + "music/pjsk/alias.json");

		if (!ifile)
		{
			logging::WARN("Unable to open alias.json <pjskMusicInfo>");
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}

		json alias = json::parse(ifile);
		ifile.close();

		for (const auto &p : alias.items())
		{
			std::unordered_set<string> s;
			for (const auto &item : p.value()["alias"].items())
			{
				string str = string(item.value());
				Utils::ToLower(str);
				Utils::ReplaceMark(str);
				s.insert(str);
			}
			alias_pair.emplace_back(p.value(), s);
		}
		for (const auto &p : alias_pair)
		{
			if (p.second.contains(target))
			{
				music = p.first;
				break;
			}
		}
	}

	if (music.is_null())
	{
		logging::INFO("未知歌曲 <pjskMusicInfo>: " + target + Utils::GetDescription(gm, false));
		client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain(target + "是什么歌捏，不知道捏"));
		return false;
	}
	int id = music["musicId"].get<int>();
	vector<string> alias =  music["alias"].get<vector<string>>();
	string title = music["title"].get<string>();
	string translate = music["translate"].get<string>();
	double length = -1;
	string cover_path;
	string cover_org_path;
	string lyricist;
	string composer;
	string arranger;
	logging::INFO("获取歌曲信息 <pjskMusicInfo>: " + title + Utils::GetDescription(gm, false));
	{
		std::ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
		
		if (!ifile)
		{
			logging::WARN("Unable to open meta.json <pjskMusicInfo>");
			client.SendGroupMessage(gm.GetSender().group.id, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}

		json meta = json::parse(ifile);
		ifile.close();

		for (const auto &p : meta.items())
		{
			if (p.value()["musicId"].get<int>() == id)
			{
				if (p.value().contains("length"))
					length = p.value()["length"].get<double>();
				cover_path = MediaFilesPath
						+ "images/pjsk/cover_small/" + p.value()["assetbundleName"].get<string>() + "_small.png";
				if (p.value()["hasOriginCover"].get<bool>())
					cover_org_path = MediaFilesPath
						+ "images/pjsk/cover_small/" + p.value()["assetbundleName"].get<string>() + "_org_small.png";

				lyricist = p.value()["lyricist"].get<string>();
				composer = p.value()["composer"].get<string>();
				arranger = p.value()["arranger"].get<string>();
				break;
			}
		}
	}
	string msg = 	"歌曲id: " + std::to_string(id)
			+ "\n曲名: " + title + ((translate.empty()) ? "" : "      译名: " + translate)
			+ "\n作词: " + lyricist + "      作曲: " + composer + "      编曲: " + arranger
			+ ((length < 0)? "" : "\n时长: " + std::to_string(int(length)) + 's')
			+ "\n其它名称:";
	for (const auto& s : alias)
	{
		msg += " 「" + s + "」";
	}

	Mirai::MessageChain m = Mirai::MessageChain().Plain(msg);

	if (!cover_path.empty() && std::filesystem::exists(cover_path))
	{
		m += Mirai::PlainMessage("\n");
		m.Image("", "", cover_path, "");
	}
	if (!cover_org_path.empty() && std::filesystem::exists(cover_org_path))
	{
		m += Mirai::PlainMessage("\n");
		m.Image("", "", cover_org_path, "");
	}
	client.SendGroupMessage(gm.GetSender().group.id, m);
	
	return true;
}

}