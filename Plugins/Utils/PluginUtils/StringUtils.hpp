#ifndef _UTILS_STRING_UTILS_HPP_
#define _UTILS_STRING_UTILS_HPP_

#include <array>
#include <exception>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <charconv>
#include <vector>

namespace Utils
{

inline bool isFloat(std::string_view str)
{
	const static std::regex r_float =
		std::regex(R"((?:\d+(?:\.\d*)?|\.\d+))", std::regex::icase);
	return std::regex_match(str.cbegin(), str.cend(), r_float);
}

inline bool isInt(std::string_view str)
{
	const static std::regex r_int = std::regex(R"([\+-]?\d+)", std::regex::icase);
	return std::regex_match(str.cbegin(), str.cend(), r_int);
}

template <typename T>
inline bool Str2Num(std::string_view str, T& value, int base = 10)	// NOLINT(*-avoid-magic-numbers)
{
	auto result = std::from_chars(str.data(), str.data() + str.size(), value, base);
	if (result.ec != std::errc{})
		return false;
	return true;
}

constexpr std::string_view trim(std::string_view str, std::string_view whitespace = " ")
{
	const auto begin = str.find_first_not_of(whitespace);
	if (begin == std::string_view::npos) return ""; // no content
	const auto end = str.find_last_not_of(whitespace);
	return str.substr(begin, end - begin + 1);
}

inline std::string ReplaceMark(std::string str)
{
	using std::string_view;
	using std::string;

	constexpr std::array<std::pair<string_view, string_view>, 12> MarkList{{
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
	for (const auto& p : MarkList)
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
			current = next + static_cast<long>(p.first.length());
			next = std::search(current, end, p.first.begin(), p.first.end());
		}
		temp.append(current, next);
		str.swap(temp);
	}
	return str;
}

inline std::string toLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
	return str;
}

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

inline size_t Tokenize(const std::string& input, std::vector<std::string>& tokens, size_t max_count = 0)
{
	std::istringstream iss(input);
	std::string s;

	if (max_count > 0) tokens.reserve(max_count);

	while (iss >> std::quoted(s))
	{
		if (max_count > 0 && tokens.size() >= max_count) tokens[max_count - 1] += ' ' + s;
		else
			tokens.push_back(s);
	}

	return tokens.size();
}

} // namespace Utils

#endif