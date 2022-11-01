#ifndef _IMAGE_SEARCH_SAUCENAO_MODELS_HPP_
#define _IMAGE_SEARCH_SAUCENAO_MODELS_HPP_

#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json_fwd.hpp>

namespace SauceNAO
{

struct SauceNAOError : public std::runtime_error
{
	const std::string message;
	const int status;

	SauceNAOError(std::string message, int status) : 
	std::runtime_error("Error response from SauceNAO: " + message + "<" + std::to_string(status) + ">"),
	message(std::move(message)), status(status)
	{}
};

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

		uint32_t ResultsExpected;	// should be integer
		std::string SearchDepth;		// should be integer
		double MinSimilarity{};

		std::string QueryImgDisplay;

		uint32_t ResultsReturned{};

		// `index`, `query_image` are ignored

		friend void from_json(const nlohmann::json& j, Header& p);

	} header{};

	struct Result
	{
		struct Header
		{
			std::string similarity;		// Should be float
			std::string thumbnail;
			uint8_t index{};
			std::string IndexName{};
			uint32_t dupes{};
			int hidden{};		// Should be bool

			friend void from_json(const nlohmann::json& j, Header& p);
		} header{};

		
		struct ResultInfo
		{
			std::string title;
			std::string author;
			std::string url;
			std::map<std::string, std::string> extras;
		} data{};

		friend void from_json(const nlohmann::json& j, Result& p);
	};

	std::vector<Result> results;

	friend void from_json(const nlohmann::json& j, SauceNAOResult& p);
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
_DECLARE_MASK_(KONACHAN, 26);
_DECLARE_MASK_(SANKAKU, 27);
_DECLARE_MASK_(ANIME_PIC, 28);
_DECLARE_MASK_(E621, 29);
_DECLARE_MASK_(IDOL_COMPLEX, 30);
_DECLARE_MASK_(BCY_ILLUST, 31);
_DECLARE_MASK_(BCY_COSPLAY, 32);
_DECLARE_MASK_(PORTAL_GRAPHICS, 33);
_DECLARE_MASK_(DEVIANT_ART, 34);
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
						| H_ANIME | MOVIES | SHOWS | GELBOORU | KONACHAN
						| SANKAKU | ANIME_PIC | E621 | IDOL_COMPLEX
						| BCY_ILLUST | BCY_COSPLAY | PORTAL_GRAPHICS | DEVIANT_ART
						| PAWOO | MADOKAMI | MANGADEX | EHENTAI_HMISC
						| ART_STATION | FUR_AFFINITY | TWITTER | FURRY_NETWORK 
						| KEMONO | SKEB ;


constexpr std::array<std::string_view, Mask2Index(SKEB) + 1> INDEX_NAME = {
	"H-Magazines",
	"H-Anime*",
	"H-Game CG",
	"ddb-objects*",
	"ddb-samples*",
	"Pixiv",
	"Pixiv - Historical",
	"anime*",
	"Nico Nico Seiga",
	"Danbooru",
	"drawr Images",
	"Nijie Images",
	"Yande.re",
	"animeop*",
	"IMDb*",
	"Shutterstock*",
	"FAKKU",
	"reserved",
	"H-MISC (NHentai)",
	"2D-Market",
	"MediBang",
	"Anime",
	"H-Anime",
	"Movies",
	"Shows",
	"Gelbooru",
	"Konachan",
	"Sankaku Channel",
	"Anime-Pictures",
	"e621",
	"Idol Complex",
	"bcy Illust",
	"bcy Cosplay",
	"PortalGraphics",
	"deviantArt",
	"Pawoo",
	"Madokami (Manga)",
	"MangaDex",
	"H-Misc (E-Hentai)",
	"ArtStation",
	"FurAffinity",
	"Twitter",
	"Furry Network",
	"Kemono",
	"Skeb"
};

}

#endif