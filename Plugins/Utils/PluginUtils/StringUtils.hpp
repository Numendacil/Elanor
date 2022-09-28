#ifndef _UTILS_STRING_UTILS_HPP_
#define _UTILS_STRING_UTILS_HPP_

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace Utils
{

bool isFloat(std::string_view str);
bool isInt(std::string_view str);

constexpr std::string_view trim(std::string_view str, std::string_view whitespace = " ")
{
	const auto begin = str.find_first_not_of(whitespace);
	if (begin == std::string_view::npos) return ""; // no content
	const auto end = str.find_last_not_of(whitespace);
	return str.substr(begin, end - begin + 1);
}

std::string ReplaceMark(std::string str);
std::string toLower(std::string str);

class UnknownInput : public std::runtime_error
{
public:
	explicit UnknownInput(const std::string& input)
	: std::runtime_error("Unknown input: " + input) {}
};

// Lowercase input only
constexpr bool toBool(std::string_view str)
{
	constexpr std::array TrueStr = {"1", "true", "on", "yes"};
	constexpr std::array FalseStr = {"0", "false", "off", "no"};
	static_assert(TrueStr.size() == FalseStr.size());
	for (int i = 0; i < TrueStr.size(); i++)
	{
		if (str == std::string_view(TrueStr.at(i))) return true;
		if (str == std::string_view(FalseStr.at(i))) return false;
	}
	throw UnknownInput(std::string(str));
}

size_t Tokenize(const std::string& input, std::vector<std::string>& tokens, size_t max_count = 0);

} // namespace Utils

#endif