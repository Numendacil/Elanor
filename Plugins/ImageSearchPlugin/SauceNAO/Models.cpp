#include "Models.hpp"

#include <vector>
#include <nlohmann/json.hpp>
#include <PluginUtils/Common.hpp>

using json = nlohmann::json;
using std::string;

using namespace std::literals;

namespace SauceNAO
{

namespace
{

string GetTitle(const json& data)
{
	constexpr std::array<std::string_view, 6> CANDIDATES = {
		"title",
		"jp_name",
		"eng_name",
		"material",
		"source",
		"created_at"
	};

	try
	{
		for (const auto& key : CANDIDATES)
		{
			auto it = data.find(key);
			if (it != data.end())
			{
				if (it->is_array())
					return (*it)[0].get<string>();
				return it->get<string>();
			}
		}
	}
	catch(const json::type_error& e)
	{
		return {};
	}
	return {};
}

string GetAuthor(const json& data)
{
	constexpr std::array<std::string_view, 7> CANDIDATES = {
		"author",
		"author_name",
		"member_name",
		"pawoo_user_username",
		"company",
		"creator",
		"twitter_user_handle"
	};
	try
	{
		for (const auto& key : CANDIDATES)
		{
			auto it = data.find(key);
			if (it != data.end())
			{
				if (it->is_array())
				{
					string author;
					bool first = true;
					for (const auto& item : *it)
					{
						if (first)
						{
							first = false;
							author += item.get<string>();
						}
						else
							author += ", " + item.get<string>();
					}
					return author;
				}
				return it->get<string>();
			}
		}
	}
	catch(const json::type_error& e)
	{
		return {};
	}
	return {};
}

string GetUrl(const json& data)
{
	constexpr std::array<std::string_view, 1> CANDIDATES = {
		"ext_urls"
	};

	try
	{
		for (const auto& key : CANDIDATES)
		{
			auto it = data.find(key);
			if (it != data.end())
			{
				if (it->is_array())
					return (*it)[0].get<string>();
				return it->get<string>();
			}
		}
	}
	catch(const json::type_error& e)
	{
		return {};
	}
	return {};
}

SauceNAOResult::Result::ResultInfo GetInfo(const json& data, uint8_t index)
{
	switch(Index2Mask(index))
	{
	case PIXIV:
	case PIXIV_HISTORY:
	{
		uint64_t id = Utils::GetValue(data, "pixiv_id", 0ULL);
		uint64_t member_id = Utils::GetValue(data, "member_id", 0ULL);
		return {
			.title =  GetTitle(data)
			+ " (id: " + (id ? std::to_string(id) : "***"s)+ ")",
			.author = GetAuthor(data)
			+ " (id: " + (member_id ? std::to_string(member_id) : "***"s) + ")",
			.url = "https://www.pixiv.net/artworks/" + (id ? std::to_string(id) : "***")
		};
	}

	case SEIGA_ILLUST:
	{
		uint64_t id = Utils::GetValue(data, "seiga_id", 0ULL);
		uint64_t member_id = Utils::GetValue(data, "member_id", 0ULL);
		return {
			.title = GetTitle(data)
			+ " (id: " + (id ? std::to_string(id) : "***"s)+ ")",
			.author = GetAuthor(data)
			+ " (id: " + (member_id ? std::to_string(member_id) : "***"s) + ")",
			.url = GetUrl(data),
		};
	}

	case DANBOORU:
	case YANDE_RE:
	case GELBOORU:
	case KONACHAN:
	case E621:
	{
		uint64_t id = Utils::GetValue(data, "danbooru_id", 0ULL);
		std::map<string, string> extras;

		auto it = data.find("source");
		if (it != data.end() && it->is_string())
			extras.emplace("原网址", it->get<string>());

		it = data.find("characters");
		if (it != data.end() && it->is_string())
			extras.emplace("人物", it->get<string>());

		return {
			.title = GetTitle(data)
			+ " (id: " + (id ? std::to_string(id) : "***"s)+ ")",
			.author = GetAuthor(data),
			.url = GetUrl(data),
			.extras = std::move(extras)
		};
	}

	case ANIME:
	case H_ANIME:
	case MOVIES:
	case SHOWS:
	{
		std::map<string, string> extras;
		auto it = data.find("year");
		if (it != data.end() && it->is_string())
			extras.emplace("年份", it->get<string>());

		it = data.find("est_time");
		if (it != data.end() && it->is_string())
		{
			auto pos = it->get<string>();
			it = data.find("part");
			if (it != data.end() && it->is_string())
				pos += "   part " + it->get<string>();
			extras.emplace("位置", std::move(pos));
		}

		return {
			.title = GetTitle(data),
			.url = GetUrl(data),
			.extras = std::move(extras)
		};
	}

	case TWITTER:
	{
		string user_id = Utils::GetValue(data, "twitter_user_id", "");
		return {
			.title = Utils::GetValue(data, "created_at", ""),
			.author = Utils::GetValue(data, "twitter_user_handle", "")
			+ " (id: " + user_id + ")",
			.url = GetUrl(data),
		};
	}

	default:
		return {
			.title = GetTitle(data),
			.author = GetAuthor(data),
			.url = GetUrl(data),
		};
	}
}

}

void from_json(const json& j, SauceNAOResult::Header& p)
{
	j.at("user_id").get_to(p.UserId);
	j.at("account_type").get_to(p.AccountType);

	j.at("short_limit").get_to(p.ShortLimit);
	j.at("long_limit").get_to(p.LongLimit);
	j.at("short_remaining").get_to(p.ShortRemaining);
	j.at("long_remaining").get_to(p.LongRemaining);

	j.at("status").get_to(p.status);

	j.at("results_requested").get_to(p.ResultsExpected);
	j.at("search_depth").get_to(p.SearchDepth);
	j.at("minimum_similarity").get_to(p.MinSimilarity);

	j.at("query_image_display").get_to(p.QueryImgDisplay);

	j.at("results_returned").get_to(p.ResultsReturned);
}

void from_json(const json& j, SauceNAOResult::Result::Header& p)
{
	j.at("similarity").get_to(p.similarity);
	j.at("thumbnail").get_to(p.thumbnail);
	j.at("index_id").get_to(p.index);
	j.at("index_name").get_to(p.IndexName);
	j.at("dupes").get_to(p.dupes);
	j.at("hidden").get_to(p.hidden);
}

void from_json(const json& j, SauceNAOResult::Result& p)
{
	j.at("header").get_to(p.header);

	p.data = GetInfo(j.at("data"), p.header.index);
}

void from_json(const json& j, SauceNAOResult& p)
{
	j.at("header").get_to(p.header);
	j.at("results").get_to(p.results);
}

}