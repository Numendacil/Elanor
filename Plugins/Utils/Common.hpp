#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <optional>
#include <random>

namespace httplib
{

class Client;
class Result;

} // namespace httplib

namespace Mirai
{

class GroupMessageEvent;
class FriendMessageEvent;
class MessageChain;

} // namespace Mirai

namespace Utils
{

std::string exec(const std::vector<std::string>& cmd);

std::string ReplaceMark(std::string str);
std::string ToLower(std::string str);
constexpr int ToBool(std::string_view str);
int Tokenize(const std::string& input, std::vector<std::string>& tokens, int max_count = -1);

bool CheckHttpResponse(const httplib::Result& result, const std::string& Caller = "");
void SetClientOptions(httplib::Client& cli);

std::string GetDescription(const Mirai::GroupMessageEvent&, bool = true);
std::string GetDescription(const Mirai::FriendMessageEvent&, bool = true);

std::string GetText(const Mirai::MessageChain& msg);

} // namespace Utils

#endif
