#include "StringUtils.hpp"

#include <array>
#include <exception>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using std::string;
using std::string_view;
using std::vector;
using namespace std::literals;

namespace Utils
{

namespace
{

const static std::regex r_float =
	std::regex(R"([\+-]?(((\d*\.\d+)|(\d+(\.(0*))?))(e[\+-]?((\d*\.\d+)|(\d+(\.(0*))?)))?))", std::regex::icase);
const static std::regex r_int = std::regex(R"([\+-]?((\d+(\.(0*))?)(e[\+]?\d+)?))", std::regex::icase);

} // namespace

bool isFloat(string_view str)
{
	return std::regex_match(str.cbegin(), str.cend(), r_float);
}

bool isInt(string_view str)
{
	return std::regex_match(str.cbegin(), str.cend(), r_int);
}


string ReplaceMark(string str)
{
	constexpr std::array<std::pair<string_view, string_view>, 12> MarkList{{{"﹟", "#"},
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
	                                                                        {"：", ":"}}};
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

string toLower(string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
	return str;
}

size_t Tokenize(const string& input, vector<string>& tokens, size_t max_count)
{
	std::istringstream iss(input);
	string s;

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