#include "Models.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>

#include <PluginUtils/Common.hpp>

using json = nlohmann::json;

namespace Pixiv
{

// *********************************************
// ************ ENUM DEFINITIONS ***************
// *********************************************

#define ENUM_TO_STR_FUNCNAME(_enum_) _##_enum_##_TO_STR_ 
#define STR_TO_ENUM_FUNCNAME(_enum_) _STR_TO_##_enum_##_
#define ENUM_STR_ARRAY(_enum_) _enum_##Str

#define DECLARE_ENUM_STR(_enum_, ...)	\
namespace	\
{	\
constexpr std::array<std::string_view, static_cast<size_t>(_enum_::UNKNOWN)> ENUM_STR_ARRAY(_enum_) = {__VA_ARGS__};	\
constexpr std::string_view ENUM_TO_STR_FUNCNAME(_enum_)(const _enum_& m)	\
{	\
	auto i = static_cast<std::size_t>(m);	\
	if (i < ENUM_STR_ARRAY(_enum_).size()) return ENUM_STR_ARRAY(_enum_).at(i);	\
	else return "";	\
}	\
constexpr _enum_ STR_TO_ENUM_FUNCNAME(_enum_)(std::string_view s)	\
{	\
	for (std::size_t i = 0; i < ENUM_STR_ARRAY(_enum_).size(); i++)	\
		if (ENUM_STR_ARRAY(_enum_).at(i) == s) return static_cast<_enum_>(i);	\
	return _enum_::UNKNOWN;	\
}	\
}

#define ENUM_TO_STR(_enum_, _input_) ENUM_TO_STR_FUNCNAME(_enum_)(_input_)
#define STR_TO_ENUM(_enum_, _input_) STR_TO_ENUM_FUNCNAME(_enum_)(_input_)



DECLARE_ENUM_STR(ContentType, "illust", "manga", "ugoira", "novel");

void from_json(const json& j, ContentType& p)
{
	p = STR_TO_ENUM(ContentType, j.get<std::string>());
}

void to_json(json& j, const ContentType& p)
{
	j = ENUM_TO_STR(ContentType, p);
}

std::string to_string(ContentType p)
{
	return std::string(ENUM_TO_STR(ContentType, p));
}


DECLARE_ENUM_STR(ImageSize, "square_medium", "medium", "large", "original");

void from_json(const json& j, ImageSize& p)
{
	p = STR_TO_ENUM(ImageSize, j.get<std::string>());
}

void to_json(json& j, const ImageSize& p)
{
	j = ENUM_TO_STR(ImageSize, p);
}


void from_json(const json& j, X_RESTRICT& p)
{
	size_t i = j.get<size_t>();
	p = (i >=0 && i < static_cast<size_t>(X_RESTRICT::UNKNOWN))? 
	static_cast<X_RESTRICT>(i) : X_RESTRICT::UNKNOWN;
}

void to_json(json& j, const X_RESTRICT& p)
{
	j = static_cast<size_t>(p);
}


DECLARE_ENUM_STR(RESTRICT, "public", "private", "all");

void from_json(const json& j, RESTRICT& p)
{
	p = STR_TO_ENUM(RESTRICT, j.get<std::string>());
}

void to_json(json& j, const RESTRICT& p)
{
	j = ENUM_TO_STR(RESTRICT, p);
}

std::string to_string(RESTRICT p)
{
	return std::string(ENUM_TO_STR(RESTRICT, p));
}


DECLARE_ENUM_STR(SortOrder, "date_desc", "date_asc", "popular_desc");

std::string to_string(SortOrder p)
{
	return std::string(ENUM_TO_STR(SortOrder, p));
}


DECLARE_ENUM_STR(SearchDuration, "within_last_day", "within_last_week", "within_last_month");

std::string to_string(SearchDuration p)
{
	return std::string(ENUM_TO_STR(SearchDuration, p));
}


DECLARE_ENUM_STR(SearchOption, "partial_match_for_tags", "exact_match_for_tags", "title_and_caption", "keyword");

std::string to_string(SearchOption p)
{
	return std::string(ENUM_TO_STR(SearchOption, p));
}


DECLARE_ENUM_STR(RankingMode, 
	"day",
	"week",
	"month",
	"day_male",
	"day_female",
	"week_original",
	"week_rookie",
	"day_manga",
	"day_r18",
	"day_male_r18",
	"day_female_r18",
	"week_r18",
	"week_r18g"
);

std::string to_string(RankingMode p)
{
	return std::string(ENUM_TO_STR(RankingMode, p));
}

// *********************************************
// ********** ENUM DEFINITIONS END *************
// *********************************************


void from_json(const json& j, User& p)
{
	p.id = Utils::GetValue(j, "id", PUID_t{});
	p.name = Utils::GetValue(j, "name", "");
	p.account = Utils::GetValue(j, "account", "");
	p.isFollowed = Utils::GetValue(j, "is_followed", false);

	p.ProfileImageUrls.clear();
	for (const auto& item : j.at("profile_image_urls").items())
		p.ProfileImageUrls[STR_TO_ENUM(ImageSize, item.key())] = item.value().get<std::string>();
}

void from_json(const json& j, Tag& p)
{
	p.name = Utils::GetValue(j, "name", "");
	p.translate = Utils::GetValue(j, "translated_name", "");
}

void from_json(const json& j, Illust& p)
{
	p.id = Utils::GetValue(j, "id", PID_t{});
	p.title = Utils::GetValue(j, "title", "");
	p.caption = Utils::GetValue(j, "caption", "");
	p.type = Utils::GetValue(j, "type", ContentType::UNKNOWN);

	p.CoverUrl.clear();
	for (const auto& item : j.at("image_urls").items())
		p.CoverUrl.emplace(STR_TO_ENUM(ImageSize, item.key()), item.value().get<std::string>());
	p.PageCount = Utils::GetValue(j, "page_count", 0);
	p.PageUrls.clear();
	if (p.PageCount < 2)
	{
		ImageUrls urls;
		if (Utils::HasValue(j, "meta_single_page"))
			urls.emplace(ImageSize::ORIGINAL, j.at("meta_single_page").at("original_image_url").get<std::string>());
		for (const auto& url : p.CoverUrl)
			urls.emplace(url);

		p.PageUrls.emplace_back(std::move(urls));
	}
	else
	{
		for (const auto& page : Utils::GetValue(j, "meta_pages", json::array()))
		{
			ImageUrls urls;
			for (const auto& item : page.at("image_urls").items())
				urls.emplace(STR_TO_ENUM(ImageSize, item.key()), item.value().get<std::string>());

			p.PageUrls.emplace_back(std::move(urls));
		}
	}
	p.width = Utils::GetValue(j, "width", 0);
	p.height = Utils::GetValue(j, "height", 0);

	p.user = Utils::GetValue(j, "user", User{});

	if (Utils::HasValue(j, "create_date"))
	{
		std::stringstream ss(j.at("create_date").get<std::string>());
		std::tm tm{};
		ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S+09:00");
		p.CreateDate = std::chrono::system_clock::from_time_t(std::mktime(&tm));
	}

	p.restrict = static_cast<RESTRICT>(Utils::GetValue(j, "restrict", size_t{}));
	p.x_restrict = Utils::GetValue(j, "x_restrict", X_RESTRICT::UNKNOWN);
	p.SanityLevel = Utils::GetValue(j, "sanity_level", 0);

	p.tags = Utils::GetValue(j, "tags", decltype(p.tags){});
	p.tools.clear();
	for (const auto& item : j.at("tools"))
	{
		p.tools.emplace_back(item.get<std::string>());
	}

	p.TotalViews = Utils::GetValue(j, "total_view", uint64_t{});
	p.TotalBookmarks = Utils::GetValue(j, "total_bookmarks", uint64_t{});
	p.isBookmarked = Utils::GetValue(j, "is_bookmarked", false);
	p.isMuted = Utils::GetValue(j, "is_muted", false);
	p.isVisible = Utils::GetValue(j, "visible", false);

	if (Utils::HasValue(j, "series") && !j.at("series").empty())
		p.series = std::make_pair(
			Utils::GetValue(j.at("series"),"id", uint64_t{}),
			Utils::GetValue(j.at("series"), "title", "")
		);
	else
		p.series = std::nullopt;

	if (Utils::HasValue(j, "total_comments"))
		p.TotalComments = Utils::GetValue(j, "total_comments", uint64_t{});
	else
		p.TotalComments = std::nullopt;
}

void from_json(const json& j, Novel& p)
{
	p.id = Utils::GetValue(j, "id", PNID_t{});
	p.title = Utils::GetValue(j, "title", "");
	p.caption = Utils::GetValue(j, "caption", "");

	p.CoverUrl.clear();
	for (const auto& item : j.at("image_urls").items())
		p.CoverUrl.emplace(STR_TO_ENUM(ImageSize, item.key()), item.value().get<std::string>());
	
	p.PageCount = Utils::GetValue(j, "page_count", 0);
	p.TextLength = Utils::GetValue(j, "text_length", uint64_t{});
	
	p.user = Utils::GetValue(j, "user", User{});

	if (Utils::HasValue(j, "create_date"))
	{
		std::stringstream ss(j.at("create_date").get<std::string>());
		std::tm tm{};
		ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S+09:00");
		p.CreateDate = std::chrono::system_clock::from_time_t(std::mktime(&tm));
	}

	p.restrict = static_cast<RESTRICT>(Utils::GetValue(j, "restrict", size_t{}));
	p.x_restrict = Utils::GetValue(j, "x_restrict", X_RESTRICT::UNKNOWN);

	p.tags.clear();
	for (const auto& item : j.at("tags"))
	{
		p.tags.emplace_back(item.get<Tag>(),
			item.at("added_by_uploaded_user").get<bool>());
	}

	p.TotalViews = Utils::GetValue(j, "total_view", uint64_t{});
	p.TotalBookmarks = Utils::GetValue(j, "total_bookmarks", uint64_t{});
	p.TotalComments = Utils::GetValue(j, "total_comments", uint64_t{});
	p.isBookmarked = Utils::GetValue(j, "is_bookmarked", false);
	p.isMuted = Utils::GetValue(j, "is_muted", false);
	p.isVisible = Utils::GetValue(j, "visible", false);
	p.isOriginal = Utils::GetValue(j, "is_original", false);
	p.isMyPixivOnly = Utils::GetValue(j, "is_mypixiv_only", false);
	p.isXRestricted = Utils::GetValue(j, "is_x_restricted", false);

	if (Utils::HasValue(j, "series") && !j.at("series").empty())
		p.series = std::make_pair(
			Utils::GetValue(j.at("series"),"id", uint64_t{}),
			Utils::GetValue(j.at("series"), "title", "")
		);
	else
		p.series = std::nullopt;

}

void from_json(const json& j, Comment& p)
{
	p.comment = Utils::GetValue(j, "comment", "");
	if (Utils::HasValue(j, "date"))
	{
		std::stringstream ss(j.at("date").get<std::string>());
		std::tm tm{};
		ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S+09:00");
		p.date = std::chrono::system_clock::from_time_t(std::mktime(&tm));
	}
	p.id = Utils::GetValue(j, "id", uint64_t{});
	p.user = Utils::GetValue(j, "user", User{});

	if (Utils::HasValue(j, "parent_comment") && !j.at("parent_comment").empty())
	{
		p.ParentComment = std::make_unique<Comment>(j.at("parent_comment").get<Comment>());
	}
}

}