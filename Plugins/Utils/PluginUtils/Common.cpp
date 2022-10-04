#include "Common.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>

#include <httplib.h>

#include <libmirai/Types/BasicTypes.hpp>
#include <libmirai/mirai.hpp>
#include <libmirai/Client.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/States/AccessCtrlList.hpp>
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
		param.push_back(const_cast<char*>(str.c_str())); // NOLINT(*-const-cast)
	param.push_back(nullptr);

	pid_t pid{};
	int p[2]; // NOLINT(*-avoid-c-arrays)
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

	constexpr size_t BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE]; // NOLINT(*-avoid-c-arrays)
	size_t c{};
	while ((c = read(p[0], buffer, BUFFER_SIZE)) > 0)
		result.append(buffer, c);
	close(p[0]);
	return result;
}


bool CheckHttpResponse(const httplib::Result& result, const string& Caller)
{
	if (result.error() != httplib::Error::Success || !result)
	{
		LOG_WARN(Utils::GetLogger(), "Connection to server failed <" + Caller + ">: " + to_string(result.error()));
		return false;
	}
	if (result->status != 200) // NOLINT(*-avoid-magic-numbers)
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
	if (result->status != 200) // NOLINT(*-avoid-magic-numbers)
	{
		LOG_WARN(Utils::GetLogger(), "Error response from server <" + Caller + ">: " + result->body);
		code = result->status;
		return false;
	}
	code = 200; // NOLINT(*-avoid-magic-numbers)
	return true;
}


void SetClientOptions(httplib::Client& cli)
{
	cli.set_compress(true);
	cli.set_decompress(true);
	cli.set_connection_timeout(300); // NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(300);       // NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(120);      // NOLINT(*-avoid-magic-numbers)
	cli.set_keep_alive(true);
}


bool CheckAuth(const Mirai::GroupMember& member, const Bot::Group& group, int permission)
{
	auto ACList = group.GetState<State::AccessCtrlList>();

	if (ACList->GetSuid() == member.id) return true;
	if (ACList->IsBlackList(member.id)) return false;

	int auth = 0;
	switch (member.permission)
	{
	case Mirai::PERMISSION::OWNER:
		auth = 30; // NOLINT(*-avoid-magic-numbers)
		break;
	case Mirai::PERMISSION::ADMINISTRATOR:
		auth = 20; // NOLINT(*-avoid-magic-numbers)
		break;
	case Mirai::PERMISSION::MEMBER:
		auth = 10; // NOLINT(*-avoid-magic-numbers)
		break;
	default:
		auth = 0; // NOLINT(*-avoid-magic-numbers)
		break;
	}

	if (ACList->IsWhiteList(member.id)) auth = 50; // NOLINT(*-avoid-magic-numbers)

	return auth >= permission;
}


string GetDescription(const Mirai::GroupMember& member, bool from)
{
	string member_str = member.MemberName + "(" + member.id.to_string() + ")";
	string group_str = member.group.name + "(" + member.group.id.to_string() + ")";
	return ((from) ? "\t<- [" : "\t-> [") + member_str + ", " + group_str + "]";
}

string GetDescription(const Mirai::User& user, bool from)
{
	string profile_str = user.nickname + "(" + user.id.to_string() + ")";
	return ((from) ? "\t<- [" : "\t-> [") + profile_str + "]";
}


string GetText(const Mirai::MessageChain& msg)
{
	string text;
	for (const auto& p : msg)
	{
		if (p->GetType() == Mirai::PlainMessage::_TYPE_)
		{
			text += static_cast<Mirai::PlainMessage*>(p.get())->GetText(); // NOLINT(*-static-cast-downcast)
		}
	}
	return text;
}
} // namespace Utils