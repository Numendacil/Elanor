#ifndef _IMAGE_SEARCH_SAUCENAO_MODELS_HPP_
#define _IMAGE_SEARCH_SAUCENAO_MODELS_HPP_

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace SauceNAO
{

struct SauceNAOResult
{
	struct Header
	{
		std::string UserId;		// should be integer
		std::string AccountType;		// should be integer/enum

		std::string ShortLimit;		// should be integer
		std::string LongLimit;		// should be integer
		uint32_t ShortRemaining{};
		uint32_t LongRemaining{};

		int32_t status{};

		std::string ResultsExpected;	// should be integer
		std::string SearchDepth;		// should be integer
		float MinSimularity{};

		std::string QueryImgDisplay;

		uint32_t ResultsReturned{};

		// `index`, `query_image` are ignored

		friend void from_json(const nlohmann::json& j, Header& p)
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
			j.at("minimum_similarity").get_to(p.MinSimularity);

			j.at("query_image_display").get_to(p.QueryImgDisplay);

			j.at("results_returned").get_to(p.ResultsReturned);
		}

		friend void to_json(nlohmann::json& j, const Header& p)
		{
			j["user_id"] = p.UserId;
			j["account_type"] = p.AccountType;

			j["short_limit"] = p.ShortLimit;
			j["long_limit"] = p.LongLimit;
			j["short_remaining"] = p.ShortRemaining;
			j["long_remaining"] = p.LongRemaining;

			j["status"] = p.status;

			j["results_requested"] = p.ResultsExpected;
			j["search_depth"] = p.SearchDepth;
			j["minimum_similarity"] = p.MinSimularity;

			j["query_image_display"] = p.QueryImgDisplay;

			j["results_returned"] = p.ResultsReturned;
		}

	} header{};

	struct Result
	{
		struct Header
		{
			std::string simularity;		// Should be float
			std::string thumbnail;
			uint8_t index{};
			std::string IndexName{};
			uint32_t dupes{};
			int hidden{};		// Should be bool
			friend void from_json(const nlohmann::json& j, Header& p)
			{
				j.at("simularity").get_to(p.simularity);
				j.at("thumbnail").get_to(p.thumbnail);
				j.at("index_id").get_to(p.index);
				j.at("index_name").get_to(p.IndexName);
				j.at("dupes").get_to(p.dupes);
				j.at("hidden").get_to(p.hidden);
			}
			friend void to_json(nlohmann::json& j, const Header& p)
			{
				j["simularity"] = p.simularity;
				j["thumbnail"] = p.thumbnail;
				j["index_id"] = p.index;
				j["index_name"] = p.IndexName;
				j["dupes"] = p.dupes;
				j["hidden"] = p.hidden;
			}
		} header{};

		nlohmann::json data;
		friend void from_json(const nlohmann::json& j, Result& p)
		{

				j.at("header").get_to(p.header);
				j.at("data").get_to(p.data);
		}
		friend void to_json(nlohmann::json& j, const Result& p)
		{
			j["header"] = p.header;
			j["data"] = p.data;
		}
	};

	std::vector<Result> results;
	friend void from_json(const nlohmann::json& j, SauceNAOResult& p)
	{

			j.at("header").get_to(p.header);
			j.at("results").get_to(p.results);
	}
	friend void to_json(nlohmann::json& j, const SauceNAOResult& p)
	{
		j["header"] = p.header;
		j["results"] = p.results;
	}
};


inline constexpr uint64_t Index2Mask(uint8_t index)
{
	// NOLINTNEXTLINE(*-avoid-magic-numbers)
	return 1UL << (index > 17 ? index - 1 : index);
}

inline constexpr uint8_t Mask2Index(uint64_t mask)
{
	if (!mask)
		return 0;
	uint8_t i = 0;
	while (!(mask & 0x1))
	{
		mask >>= 1;
		i++;
	}
	// NOLINTNEXTLINE(*-avoid-magic-numbers)
	return i >= 17 ? i + 1 : i;
}

#define _DECLARE_MASK_(_name_, _index_)	\
constexpr uint64_t _name_ = Index2Mask(_index_)

_DECLARE_MASK_(H_MAGS, 0);
_DECLARE_MASK_(HCG, 2);
_DECLARE_MASK_(PIXIV, 5);
_DECLARE_MASK_(PIXIV_HISTORY, 6);
_DECLARE_MASK_(SEIGA_ILLUST, 8);
_DECLARE_MASK_(DANBOORU, 9);
_DECLARE_MASK_(DRAWR, 10);
_DECLARE_MASK_(NIJIE, 11);
_DECLARE_MASK_(YANDE_RE, 12);
_DECLARE_MASK_(FAKKU, 16);
_DECLARE_MASK_(NHENTAI_HMISC, 18);
_DECLARE_MASK_(MARKET2D, 19);
_DECLARE_MASK_(MEDIBANG, 20);
_DECLARE_MASK_(ANIME, 21);
_DECLARE_MASK_(H_ANIME, 22);
_DECLARE_MASK_(MOVIES, 23);
_DECLARE_MASK_(SHOWS, 24);
_DECLARE_MASK_(GELBOORU, 25);
_DECLARE_MASK_(KANACHAN, 26);
_DECLARE_MASK_(SANKAKU, 27);
_DECLARE_MASK_(ANIME_PIC, 28);
_DECLARE_MASK_(E621, 29);
_DECLARE_MASK_(IDOL_COMPLEX, 30);
_DECLARE_MASK_(BCY_ILLUST, 31);
_DECLARE_MASK_(BCY_COSPLAY, 32);
_DECLARE_MASK_(PORTAL_GRAPHICS, 33);
_DECLARE_MASK_(DA, 34);
_DECLARE_MASK_(PAWOO, 35);
_DECLARE_MASK_(MADOKAMI, 36);
_DECLARE_MASK_(MANGADEX, 37);
_DECLARE_MASK_(EHENTAI_HMISC, 38);
_DECLARE_MASK_(ART_STATION, 39);
_DECLARE_MASK_(FUR_AFFINITY, 40);
_DECLARE_MASK_(TWITTER, 41);
_DECLARE_MASK_(FURRY_NETWORK, 42);
_DECLARE_MASK_(KEMONO, 43);
_DECLARE_MASK_(SKEB, 44);

#undef _DECLARE_MASK_

constexpr uint64_t MASK_ALL = H_MAGS | HCG | PIXIV | PIXIV_HISTORY | SEIGA_ILLUST
						| DANBOORU | DRAWR | NIJIE | YANDE_RE | FAKKU 
						| NHENTAI_HMISC | MARKET2D | MEDIBANG | ANIME
						| H_ANIME | MOVIES | SHOWS | GELBOORU | KANACHAN
						| SANKAKU | ANIME_PIC | E621 | IDOL_COMPLEX
						| BCY_ILLUST | BCY_COSPLAY | PORTAL_GRAPHICS | DA
						| PAWOO | MADOKAMI | MANGADEX | EHENTAI_HMISC
						| ART_STATION | FUR_AFFINITY | TWITTER | FURRY_NETWORK 
						| KEMONO | SKEB ;



}

#endif