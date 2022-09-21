#include "Common.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>

#include <httplib.h>

#include <libmirai/mirai.hpp>

#include <Core/Utils/Logger.hpp>

using json = nlohmann::json;
using std::pair;
using std::string;
using std::string_view;
using std::vector;

namespace Utils
{
string exec(const vector<string>& cmd)
{
	string result = "";
	vector<char*> param;
	for (auto& str : cmd)
		param.push_back(const_cast<char*>(str.c_str()));
	param.push_back(nullptr);

	pid_t pid{};
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

	char buffer[1024];
	ssize_t c;
	while ((c = read(p[0], buffer, 1024)) > 0)
		result.append(buffer, c);
	close(p[0]);
	return result;
}

string ReplaceMark(string str)
{
	constexpr std::array<pair<string_view, string_view>, 12> ReplaceList{{
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
	}};
	for (const auto& p : ReplaceList)
	{
		string temp;
		temp.reserve(str.size());
		const auto end = str.end();
		auto current = str.begin();
		auto next = std::search(current, end, p.first.begin(), p.first.end());
		while (next != end)
		{
			temp.append(current, next);
			temp.append(p.second);
			current = next + p.first.length();
			next = std::search(current, end, p.first.begin(), p.first.end());
		}
		temp.append(current, next);
		str.swap(temp);
	}
	return str;
}

string ToLower(string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
	return str;
}

// Lowercase input only
constexpr int ToBool(string_view str)
{
	constexpr std::array TrueStr = {"1", "true", "on", "yes"};
	constexpr std::array FalseStr = {"0", "false", "off", "no"};
	static_assert(TrueStr.size() == FalseStr.size());
	for (int i = 0; i < TrueStr.size(); i++)
	{
		if (str == string_view(TrueStr.at(i))) return 1;
		if (str == string_view(FalseStr.at(i))) return 0;
	}
	return -1;
}

int Tokenize(const string& input, vector<string>& tokens, int max_count)
{
	std::istringstream iss(input);
	string s;
	int count = 0;
	while (iss >> std::quoted(s) && count != max_count)
	{
		tokens.push_back(s);
		count++;
	}
	return tokens.size();
}

bool CheckHttpResponse(const httplib::Result& result, const string& Caller)
{
	if (result.error() != httplib::Error::Success || !result)
	{
		LOG_WARN(Utils::GetLogger(), "Connection to server failed <" + Caller + ">: " + to_string(result.error()));
		return false;
	}
	if (result->status != 200)
	{
		LOG_WARN(Utils::GetLogger(), "Error response from server <" + Caller + ">: " + result->body);
		return false;
	}
	return true;
}

bool CheckHttpResponse(const httplib::Result& result, const string& Caller, int& code)
{
	if (result.error() != httplib::Error::Success || !result)
	{
		LOG_WARN(Utils::GetLogger(), "Connection to server failed <" + Caller + ">: " + to_string(result.error()));
		code = -1;
		return false;
	}
	if (result->status != 200)
	{
		LOG_WARN(Utils::GetLogger(), "Error response from server <" + Caller + ">: " + result->body);
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

string GetDescription(const Mirai::GroupMessageEvent& gm, bool from)
{
	string member = gm.GetSender().MemberName + "(" + gm.GetSender().id.to_string() + ")";
	string group = gm.GetSender().group.name + "(" + gm.GetSender().group.id.to_string() + ")";
	return ((from) ? "\t<- [" : "\t-> [") + member + ", " + group + "]";
}

string GetDescription(const Mirai::FriendMessageEvent& fm, bool from)
{
	string profile = fm.GetSender().nickname + "(" + fm.GetSender().id.to_string() + ")";
	return ((from) ? "\t<- [" : "\t-> [") + profile + "]";
}

string GetText(const Mirai::MessageChain& msg)
{
	string text;
	for (const auto& p : msg)
	{
		if (p->GetType() == Mirai::PlainMessage::_TYPE_)
		{
			text += static_cast<Mirai::PlainMessage*>(p.get())->GetText();
		}
	}
	return text;
}
} // namespace Utils