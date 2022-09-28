#ifndef _UTILS_COMMON_HPP_
#define _UTILS_COMMON_HPP_

#include <optional>
#include <random>

namespace httplib
{

class Client;
class Result;

} // namespace httplib

namespace Mirai
{

class GroupMember;
class User;
class MessageChain;

} // namespace Mirai

namespace Bot
{

class Group;

}

namespace Utils
{

std::string exec(const std::vector<std::string>& cmd);

bool CheckHttpResponse(const httplib::Result& result, const std::string& Caller = "");
void SetClientOptions(httplib::Client& cli);

bool CheckAuth(const Mirai::GroupMember& member, const Bot::Group& group, int permission);

std::string GetDescription(const Mirai::GroupMember& member, bool from = true);
std::string GetDescription(const Mirai::User& user, bool from = true);

std::string GetText(const Mirai::MessageChain& msg);

} // namespace Utils

#endif
