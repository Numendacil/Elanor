#ifndef _PIXIV_MODELS_HPP_
#define _PIXIV_MODELS_HPP_

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include <libmirai/Types/BasicTypes.hpp>

namespace Pixiv
{

// Illustration Id
class PID_t : public Mirai::UID_t
{
public:
	PID_t() : UID_t() {}
	explicit PID_t(int64_t num) : UID_t(num) {}
};

inline PID_t operator""_pid(unsigned long long num)
{
	return PID_t(static_cast<int64_t>(num));
}


// User Id
class PUID_t : public Mirai::UID_t
{
public:
	PUID_t() : UID_t() {}
	explicit PUID_t(int64_t num) : UID_t(num) {}
};

inline PUID_t operator""_puid(unsigned long long num)
{
	return PUID_t(static_cast<int64_t>(num));
}

// Novel Id
class PNID_t : public Mirai::UID_t
{
public:
	PNID_t() : UID_t() {}
	explicit PNID_t(int64_t num) : UID_t(num) {}
};

inline PNID_t operator""_pnid(unsigned long long num)
{
	return PNID_t(static_cast<int64_t>(num));
}


enum class ContentType : size_t 
{ 
	ILLUST = 0, 
	MANGA, 
	UGOIRA, 
	NOVEL, 
	UNKNOWN 
};
void from_json(const nlohmann::json& j, ContentType& p);
void to_json(nlohmann::json& j, const ContentType& p);
std::string to_string(ContentType p);

enum class ImageSize : size_t 
{ 
	SQUARE_MEDIUM  = 0, 
	MEDIUM, 
	LARGE, 
	ORIGINAL, 
	UNKNOWN 
};
void from_json(const nlohmann::json& j, ImageSize& p);
void to_json(nlohmann::json& j, const ImageSize& p);

enum class X_RESTRICT : size_t 
{ 
	SAFE  = 0, 
	R18, 
	R18G, 
	UNKNOWN 
};
void from_json(const nlohmann::json& j, X_RESTRICT& p);
void to_json(nlohmann::json& j, const X_RESTRICT& p);

enum class RESTRICT : size_t 
{ 
	PUBLIC  = 0, 
	PRIVATE, 
	ALL,
	UNKNOWN 
};
void from_json(const nlohmann::json& j, RESTRICT& p);
void to_json(nlohmann::json& j, const RESTRICT& p);
std::string to_string(RESTRICT p);

enum class SortOrder : size_t
{
	DATE_DESC = 0,
	DATE_ASC,
	POPULAR_DESC,
	UNKNOWN
};
std::string to_string(SortOrder p);

enum class SearchDuration : size_t
{
	LAST_DAY = 0,
	LAST_WEEK,
	LAST_MONTH,
	UNKNOWN
};
std::string to_string(SearchDuration p);

enum class SearchOption : size_t
{
	TAGS_PARTIAL = 0,
	TAGS_EXACT,
	TITLE_AND_CAPTION,
	KEYWORD,
	UNKNOWN
};
std::string to_string(SearchOption p);

enum class RankingMode : size_t
{
	DAY = 0,
	WEEK,
	MONTH,
	DAY_MALE,
	DAY_FEMALE,
	WEEK_ORIGINAL,
	WEEK_ROOKIE,
	DAY_MANGA,
	DAY_R18,
	DAY_MALE_R18,
	DAY_FEMALE_R18,
	WEEK_R18,
	WEEK_R18G,
	UNKNOWN
};
std::string to_string(RankingMode p);


using ImageUrls = std::map<ImageSize, std::string>;

struct User
{
	PUID_t id{};
	std::string name;
	std::string account;
	ImageUrls ProfileImageUrls;
	std::string comment;
	bool isFollowed{};
};

void from_json(const nlohmann::json& j, User& p);


struct Tag
{
	std::string name;
	std::string translate;
};

void from_json(const nlohmann::json& j, Tag& p);

struct Illust
{
	PID_t id{};
	std::string title;
	std::string caption;
	ContentType type = ContentType::UNKNOWN;

	// Urls of the cover.
	ImageUrls CoverUrl;
	// Urls of images in an illust.
	std::vector<ImageUrls> PageUrls;
	int PageCount{};
	int width{};
	int height{};

	User user;

	std::chrono::system_clock::time_point CreateDate{};

	RESTRICT restrict{};
	X_RESTRICT x_restrict = X_RESTRICT::UNKNOWN;
	int SanityLevel{};

	std::vector<Tag> tags;
	std::vector<std::string> tools;

	uint64_t TotalViews{};
	uint64_t TotalBookmarks{};
	bool isBookmarked{};
	bool isMuted{};
	bool isVisible{};

	std::optional<std::pair<uint64_t, std::string>> series; // id, title
	std::optional<uint64_t> TotalComments;

	std::vector<std::string> GetImageUrls()
	{
		std::vector<std::string> urls;
		for (const auto& p : this->PageUrls)
		{
			if (!p.empty())
				urls.push_back(p.crbegin()->second);
		}
		return urls;
	}
};

void from_json(const nlohmann::json& j, Illust& p);

struct Novel
{
	PNID_t id{};
	std::string title;
	std::string caption;

	int PageCount{};
	uint64_t TextLength{};

	// Urls of the cover.
	ImageUrls CoverUrl;

	User user;

	std::chrono::system_clock::time_point CreateDate{};

	RESTRICT restrict{};
	X_RESTRICT x_restrict = X_RESTRICT::UNKNOWN;

	std::vector<std::pair<Tag, bool>> tags;	// tag, is_added_by_uploader

	uint64_t TotalViews{};
	uint64_t TotalBookmarks{};
	uint64_t TotalComments;
	bool isBookmarked{};
	bool isMuted{};
	bool isVisible{};
	bool isOriginal{};
	bool isXRestricted{};
	bool isMyPixivOnly{};

	std::optional<std::pair<uint64_t, std::string>> series; // id, title
};

void from_json(const nlohmann::json& j, Novel& p);


struct Comment
{
	std::string comment;
	std::chrono::system_clock::time_point date{};
	uint64_t id;
	User user;

	std::unique_ptr<Comment> ParentComment;
};

void from_json(const nlohmann::json& j, Comment& p);

}

#endif