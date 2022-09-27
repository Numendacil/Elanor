#include <exception>
#include <fstream>
#include <optional>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#include <httplib.h>
#include <ThirdParty/log.h>
#include <libmirai/mirai.hpp>

#include <Command/Command.hpp>
#include <State/State.hpp>
#include <Trigger/Trigger.hpp>
#include <Utils/Utils.hpp>

using json = nlohmann::json;
using std::string;
using std::vector;
using std::pair;

namespace Utils
{
	bool BotConfig::FromFile(const string &filepath)
	{
		std::ifstream ifile(filepath);
		if (ifile.fail())
		{
			logging::WARN("Failed to open file <BotConfig>: " + filepath);
			return false;
		}
		try
		{
			this->config = json::parse(ifile);
		}
		catch (const std::exception& e)
		{
			logging::WARN("Failed to parse file <BotConfig>: " + string(e.what()));
			return false;
		}
		return true;
	}
}

namespace Utils
{
	std::random_device rd;
	std::mt19937 rng_engine(rd());
	BotConfig Configs;

	string exec(const vector<string>& cmd)
	{
		string result = "";
		vector<char *> param;
		for (auto &str : cmd)
			param.push_back(const_cast<char *>(str.c_str()));
		param.push_back(nullptr);

		pid_t pid;
		int p[2];
		if (pipe(p) == -1)
		{
			perror("Failed to open pipe");
			return nullptr;
		}
		if ((pid = fork()) == -1)
		{
			perror("Failed to fork program");
			return nullptr;
		}
		if (pid == 0)
		{
			dup2(p[1], STDOUT_FILENO);
			close(p[0]);
			close(p[1]);
			execvp(param[0], param.data());
			perror(("Failed to execute " + cmd[0]).c_str());
			exit(1);
		}
		close(p[1]);

		char buffer[128];
		ssize_t c;
		while ((c = read(p[0], buffer, 128)) > 0)
			result.append(buffer, c);
		close(p[0]);
		return result;
	}

	void ReplaceMark(string &str)
	{
		static const vector<pair<const string, const string>> ReplaceList =
		{
			{"﹟", "#"},
			{"？", "?"},
			{"＃", "#"},
			{"！", "!"},
			{"。", "."},
			{"，", ","},
			{"“", "\""},
			{"”", "\""},
			{"‘", "\'"},
			{"’", "\'"},
			{"；", ";"},
			{"：", ":"}
		};
		for (const auto &p : ReplaceList)
		{
			string temp;
			const auto end = str.end();
			auto current = str.begin();
			auto next = search(current, end, p.first.begin(), p.first.end());
			while (next != end)
			{
				temp.append(current, next);
				temp.append(p.second);
				current = next + p.first.length();
				next = search(current, end, p.first.begin(), p.first.end());
			}
			temp.append(current, next);
			str = temp;
		}
	}

	void ToLower(string &str)
	{
		transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
			  { return tolower(c); });
	}

	int ToBool(const string& str)
	{
		string s = str;
		ToLower(s);
		constexpr std::string_view TrueStr[] = {"1", "true", "on", "yes"};
		constexpr std::string_view FalseStr[] = {"0", "false", "off", "no"};
		static_assert(std::extent<decltype(TrueStr)>::value == std::extent<decltype(FalseStr)>::value);
		for (int i = 0; i < std::extent<decltype(TrueStr)>::value; i++)
		{
			if (s == TrueStr[i])
				return 1;
			if (s == FalseStr[i])
				return 0;
		}
		return -1;
	}

	int Tokenize(vector<string> &token, string str, int count)
	{
		std::istringstream iss(str);
		string s;
		int i = 0;
		while (iss >> quoted(s) && i != count)
		{
			token.push_back(s);
			i++;
		}
		return token.size();
	}

	bool CheckHttpResponse(const httplib::Result& result, const string& Caller)
	{
		if (result.error() != httplib::Error::Success || !result)
		{
			logging::WARN("Connection to server failed <" + Caller + ">: " + to_string(result.error()));
			return false;
		}
		if (result->status != 200)
		{
			logging::WARN("Error response from server <" + Caller + ">: " + result->body);
			return false;
		}
		return true;
	}

	bool CheckHttpResponse(const httplib::Result& result, const string& Caller, int& code)
	{
		if (result.error() != httplib::Error::Success || !result)
		{
			logging::WARN("Connection to server failed <" + Caller + ">: " + to_string(result.error()));
			code = -1;
			return false;
		}
		if (result->status != 200)
		{
			logging::WARN("Error response from server <" + Caller + ">: " + result->body);
			code = result->status;
			return false;
		}
		code = 200;
		return true;
	}

	void SetClientOptions(httplib::Client& cli)
	{
		cli.set_compress(true);
		cli.set_decompress(true);
		cli.set_connection_timeout(300);
		cli.set_read_timeout(300);
		cli.set_write_timeout(120);
		cli.set_keep_alive(true);
	}

	string GetDescription(const Mirai::GroupMessageEvent &gm, bool from)
	{
		string member = gm.GetSender().MemberName + "(" + gm.GetSender().id.to_string() + ")";
		string group = gm.GetSender().group.name + "(" + gm.GetSender().group.id.to_string() + ")";
		return ((from) ? "\t<- [" : "\t-> [") + member + ", " + group + "]";
	}

	string GetDescription(const Mirai::FriendMessageEvent &fm, bool from)
	{
		string profile = fm.GetSender().nickname + "(" + fm.GetSender().id.to_string() + ")";
		return ((from) ? "\t<- [" : "\t-> [") + profile + "]";
	}

	string GetText(const Mirai::MessageChain& msg)
	{
		string text;
		for(const auto& p : msg)
		{
			if (p->GetType() == Mirai::PlainMessage::_TYPE_)
			{
				text += static_cast<Mirai::PlainMessage*>(p.get())->GetText();
			}
		}
		return text;
	}
}