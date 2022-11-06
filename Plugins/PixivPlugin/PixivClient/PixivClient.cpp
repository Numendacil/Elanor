#include <chrono>
#include <ctime>

#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <vector>

#include "PixivClient.hpp"

#include <PluginUtils/NetworkUtils.hpp>
#include <PluginUtils/UrlComponents.hpp>
#include <httplib.h>

#include <Core/Utils/Logger.hpp>

#include <openssl/evp.h>

using std::string;
using json = nlohmann::json;

namespace Pixiv
{

namespace
{

httplib::Headers GetClientHashHeader()
{
	// NOLINTBEGIN

	constexpr std::string_view hash_secret = "28c1fdd170a5204386cb1313c7077b34f83e4aaf4aa829ce78c231e05b0bae2c";
	
	auto t = std::time(nullptr);
	char time_str[128];
	std::strftime(time_str, 128, "%Y-%m-%dT%H:%M:%S+00:00", std::localtime(&t));

	string hashstr = string(time_str) + string(hash_secret);

	std::unique_ptr<EVP_MD_CTX, std::function<void(EVP_MD_CTX*)>> ctx(
		EVP_MD_CTX_new(),
		EVP_MD_CTX_free
	);
	if (!ctx)
		throw std::runtime_error("Failed to create md context");

	int rc = EVP_DigestInit_ex(ctx.get(), EVP_md5(), nullptr);
	if(rc != 1)
		throw std::runtime_error("Failed to init digest");

	constexpr auto MAX_SIZE = std::numeric_limits<int>::max();
	const auto* pos = hashstr.data();
	size_t remaining = hashstr.size();
	while (remaining > MAX_SIZE)
	{
		rc = EVP_DigestUpdate(ctx.get(), pos, MAX_SIZE);
		if(rc != 1)
			throw std::runtime_error("Failed to update digest");
		remaining -= MAX_SIZE;
		pos += MAX_SIZE;
	}
	rc = EVP_DigestUpdate(ctx.get(), pos, remaining);
	if(rc != 1)
		throw std::runtime_error("Failed to update digest");

	constexpr size_t MD5_DIGEST_LENGTH = 16;
	unsigned char buffer[MD5_DIGEST_LENGTH];
	uint digest_len{};
	rc = EVP_DigestFinal_ex(
		ctx.get(), 
		buffer, 
		&digest_len
	);
	assert(digest_len == MD5_DIGEST_LENGTH);

	char result[2 * MD5_DIGEST_LENGTH + 1];
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(result + 2 * i, "%02x", buffer[i]);
	
	// NOLINTEND

	return {
		{"X-Client-Time", time_str},
		{"X-Client-Hash", result}
	};
}

}

httplib::Client PixivClient::_GetOAuthClient() const
{
	httplib::Client cli(oauth_hosts.data());
	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
		cli.set_proxy(this->_ProxyHost, this->_ProxyPort);

	cli.set_connection_timeout(10);		// NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(10);		// NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(60);		// NOLINT(*-avoid-magic-numbers)

	cli.set_default_headers({
		{"User-Agent", "PixivAndroidApp/5.0.234 (Android 11; Pixel 5)"},
		{"App-Version", "5.0.234"},
		{"App-OS-Version", "Android 11.0"},
		{"App-OS", "Android"},
		{"Accept-Language", "zh-cn"}
	});

	return cli;
}

httplib::Client PixivClient::_GetApiClient() const
{
	httplib::Client cli(api_hosts.data());
	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
		cli.set_proxy(this->_ProxyHost, this->_ProxyPort);

	cli.set_connection_timeout(10);		// NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(10);		// NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(60);		// NOLINT(*-avoid-magic-numbers)

	cli.set_default_headers({
		{"User-Agent", "PixivAndroidApp/5.0.234 (Android 11; Pixel 5)"},
		{"App-Version", "5.0.234"},
		{"App-OS-Version", "Android 11.0"},
		{"App-OS", "Android"},
		{"Accept-Language", "zh-cn"}
	});

	return cli;
}

httplib::Client PixivClient::_GetDownloadClient() const
{
	httplib::Client cli(download_hosts.data());
	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
		cli.set_proxy(this->_ProxyHost, this->_ProxyPort);

	cli.set_connection_timeout(10);		// NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(10);		// NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(120);		// NOLINT(*-avoid-magic-numbers)

	return cli;
}

void PixivClient::_login()
{
	auto headers = GetClientHashHeader();

	httplib::Params params{
		{"get_secure_url", "1"}, 
		{"client_id", client_id.data()}, 
		{"client_secret", client_secret.data()}, 
		{"grant_type", "refresh_token"},
		{"refresh_token", this->_RefreshToken},
		{"include_policy", "true"}
	};
	auto result = this->_GetOAuthClient().Post("/auth/token", headers, params);
	
	json resp = Utils::GetJsonResponse(result);

	this->_AccessToken = resp.at("access_token").get<std::string>();
	this->_ObtainedTime = std::chrono::system_clock::now();

	LOG_INFO(Utils::GetLogger(), "Login successful, Access Token: " + this->_AccessToken);
}

string PixivClient::_GetToken()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	if (this->_AccessToken.empty() || 
		std::chrono::system_clock::now() - this->_ObtainedTime > expire_timeout)
	{
		LOG_INFO(Utils::GetLogger(), "Token expired, relogging");
		this->_login();
	}

	return this->_AccessToken;
}

string PixivClient::DownloadIllust(const string& url)
{
	Utils::UrlComponent component = Utils::UrlComponent::ParseUrl(url);

	auto result = this->_GetDownloadClient().Get(
		component.path, 
		httplib::Headers{
			{"User-Agent", "PixivIOSApp/5.8.0"},
			{"Referer", api_hosts.data()}
		});

	if (!Utils::VerifyResponse(result))
		throw Utils::NetworkException(result);

	return result->body;
}

void PixivClient::DownloadIllust(const string& url, std::function<bool(const char*, size_t)> receiver)
{
	Utils::UrlComponent component = Utils::UrlComponent::ParseUrl(url);

	auto result = this->_GetDownloadClient().Get(
		component.path, 
		httplib::Headers{
			{"User-Agent", "PixivIOSApp/5.8.0"},
			{"Referer", api_hosts.data()}
		},
		std::move(receiver));

	if (!Utils::VerifyResponse(result))
		throw Utils::NetworkException(result);
}

std::vector<string> PixivClient::BatchDownloadIllust(const std::vector<string>& urls)
{
	std::vector<string> results;
	auto client = this->_GetDownloadClient();
	client.set_keep_alive(true);

	for (const auto& url : urls)
	{
		Utils::UrlComponent component = Utils::UrlComponent::ParseUrl(url);

		auto result = client.Get(
			component.path, 
			httplib::Headers{
				{"User-Agent", "PixivIOSApp/5.8.0"},
				{"Referer", api_hosts.data()}
			});

		if (!Utils::VerifyResponse(result))
		{
			if (!result)
				LOG_WARN(Utils::GetLogger(), "Failed to download illust. Error: " + httplib::to_string(result.error()));
			else
				LOG_WARN(Utils::GetLogger(), "Failed to download illust. Reason: " + result->reason + ", Body: " + result->body);
			return results;
		}

		results.emplace_back(std::move(result->body));
	}
	return results;
}

void PixivClient::BatchDownloadIllust(const std::vector<string>& urls, std::function<bool(string, size_t)> receiver)
{
	auto client = this->_GetDownloadClient();
	client.set_keep_alive(true);

	for (size_t i = 0; i < urls.size(); i++)
	{
		Utils::UrlComponent component = Utils::UrlComponent::ParseUrl(urls[i]);

		auto result = client.Get(
			component.path, 
			httplib::Headers{
				{"User-Agent", "PixivIOSApp/5.8.0"},
				{"Referer", api_hosts.data()}
			});

		if (!Utils::VerifyResponse(result))
		{
			if (!result)
				LOG_WARN(Utils::GetLogger(), "Failed to download illust. Error: " + httplib::to_string(result.error()));
			else
				LOG_WARN(Utils::GetLogger(), "Failed to download illust. Reason: " + result->reason + ", Body: " + result->body);
			break;
		}

		receiver(std::move(result->body), i);
	}
}

json PixivClient::GetUserDetails(PUID_t UserId)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"filter", "for_android"}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/detail", params, headers));
}

json PixivClient::SearchUser(string name, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"filter", "for_android"}
	};
	params.emplace("word", std::move(name));
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/search/user", params, headers));
}

json PixivClient::FollowUser(PUID_t UserId, RESTRICT restrict)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"restrict", to_string(restrict)}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Post("/v1/user/follow/add", headers, params));
}

json PixivClient::UnfollowUser(PUID_t UserId)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Post("/v1/user/follow/delete", headers, params));
}

json PixivClient::GetUserBookmarkTags(PUID_t UserId, RESTRICT restrict, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"restrict", to_string(restrict)}
	};

	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/bookmark-tags/illust", params, headers));
}

json PixivClient::GetUserIllusts(PUID_t UserId, ContentType type, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"type", to_string(type)},
		{"filter", "for_android"}
	};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/illusts", params, headers));
}

json PixivClient::GetUserNovels(PUID_t UserId, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"filter", "for_android"}
	};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/novels", params, headers));
}

json PixivClient::GetUserFollowing(PUID_t UserId, RESTRICT restrict, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"restrict", to_string(restrict)},
		{"filter", "for_android"}
	};

	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/following", params, headers));
}

json PixivClient::GetUserFollower(PUID_t UserId, RESTRICT restrict, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"restrict", to_string(restrict)},
		{"filter", "for_android"}
	};

	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/follower", params, headers));
}

json PixivClient::GetUserMyPixiv(PUID_t UserId, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"filter", "for_android"}
	};

	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/mypixiv", params, headers));
}

json PixivClient::GetUserBlacklist(PUID_t UserId, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()},
		{"filter", "for_android"}
	};

	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/user/list", params, headers));
}

json PixivClient::GetUserBookmarkedIllusts(PUID_t UserId, RESTRICT restrict, 
				PID_t MaxBookmarkId, string tag)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"user_id", UserId.to_string()}, 
		{"restrict", to_string(restrict)},
		{"filter", "for_android"}
	};
	if (MaxBookmarkId > 0_pid)
		params.emplace("max_bookmark_id", MaxBookmarkId.to_string());
	if (!tag.empty())
		params.emplace("tag", std::move(tag));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/bookmarks/illust", params, headers));
}

json PixivClient::GetRelatedUsers(PUID_t UserId, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"user_id", UserId.to_string()}};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/related", params, headers));
}

json PixivClient::GetRecommendedUser(uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"filter", "for_android"}};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/user/recommended", params, headers));
}

json PixivClient::GetFollowedIllust(RESTRICT restrict, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"restrict", to_string(restrict)}};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/illust/follow", params, headers));
}

json PixivClient::GetIllustDetails(PID_t pid)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"illust_id", pid.to_string()}};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/illust/detail", params, headers));
}

json PixivClient::GetIllustComments(PID_t pid, uint64_t offset, bool IncludeTotalComments)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"illust_id", pid.to_string()}};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));
	if (IncludeTotalComments)
		params.emplace("include_total_comments", "true");

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/illust/comments", params, headers));
}

json PixivClient::GetRelatedIllusts(PID_t pid, uint64_t offset, const std::vector<PID_t>& seeds)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"illust_id", pid.to_string()}};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));
	if (!seeds.empty())
	{
		for (const auto& id : seeds)
			params.emplace("seed_illust_ids[]", id.to_string());
	}

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/illust/related", params, headers));
}

json PixivClient::GetRecommendedIllusts(ContentType type, 
	bool IncludeRankingIllusts, bool IncludeRankingLabel, 
	PID_t MinBookmarkId, PID_t MaxBookmarkId,
	uint64_t offset, 
	const std::vector<PID_t>& bookmark_illust_ids,
	const std::vector<PID_t>& viewed_ids,
	bool include_privacy_policy)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"content_type", to_string(type)}, 
		{"include_ranking_label", IncludeRankingLabel ? "true" : "false"},
		{"filter", "for_ios"}
	};
	if (IncludeRankingIllusts)
		params.emplace("include_ranking_illusts", "true");
	if (MinBookmarkId > 0_pid)
		params.emplace("min_bookmark_id_for_recent_illust", MinBookmarkId.to_string());
	if (MaxBookmarkId > 0_pid)
		params.emplace("max_bookmark_id_for_recommend", MaxBookmarkId.to_string());
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));
	if (!bookmark_illust_ids.empty())
	{
		for (const auto& id : bookmark_illust_ids)
			params.emplace("bookmark_illust_ids[]", id.to_string());
	}
	if (!viewed_ids.empty())
	{
		for (const auto& id : viewed_ids)
			params.emplace("viewed[]", id.to_string());
	}
	if (include_privacy_policy)
		params.emplace("include_privacy_policy", "true");

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/illust/recommended", params, headers));
}

json PixivClient::GetIllustRanking(RankingMode option, std::string date, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"mode", to_string(option)}};
	if (!date.empty())
		params.emplace("date", std::move(date));
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/illust/ranking", params, headers));
}

json PixivClient::GetTrendingTagsIllust()
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"filter", "for_android"}};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/trending-tags/illust", params, headers));
}

json PixivClient::GetTrendingTagsNovel()
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"filter", "for_android"}};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/trending-tags/novel", params, headers));
}

json PixivClient::SearchIllust(std::string keyword, SearchOption option, SortOrder sort,
			SearchDuration duration, std::string start_date, std::string end_state, 
			bool MergePlainKeywordResult, uint64_t offset)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"search_target", to_string(option)},
		{"sort", to_string(sort)},
		{"filter", "for_android"}
	};
	params.emplace("word", std::move(keyword));
	if (MergePlainKeywordResult)
		params.emplace("merge_plain_keyword_results", "true");
	if (duration != SearchDuration::UNKNOWN)
		params.emplace("duration", to_string(duration));
	if (!start_date.empty())
		params.emplace("start_date", std::move(start_date));
	if (!end_state.empty())
		params.emplace("end_state", std::move(end_state));
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/search/illust", params, headers));
}

json PixivClient::GetIllustBookmarkDetails(PID_t pid)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{{"illust_id", pid.to_string()}};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/illust/bookmark/detail", params, headers));
}

json PixivClient::AddBookmark(PID_t pid, RESTRICT restrict, const std::vector<std::string>& tags)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"illust_id", pid.to_string()},
		{"restrict", to_string(restrict)}
	};

	if (!tags.empty())
	{
		string str = std::accumulate(tags.begin(), tags.end(), string{},
		[](const string& acc, string s){ return acc.empty()? s : acc + " " + s;});
		params.emplace("tags[]", str);
	}

	return Utils::GetJsonResponse(this->_GetApiClient().Post("/v2/illust/bookmark/add", headers, params));
}

json PixivClient::DeleteBookmark(PID_t pid)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"illust_id", pid.to_string()}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Post("/v2/illust/bookmark/delete", headers, params));
}

json PixivClient::GetUgoiraMetadata(PID_t pid)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"illust_id", pid.to_string()}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/ugoira/metadata", params, headers));
}

json PixivClient::GetNovelSeries(uint64_t SeriesId, std::string LastOrder)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"series_id", std::to_string(SeriesId)},
		{"filter", "for_android"}
	};

	if (!LastOrder.empty())
		params.emplace("last_order", std::move(LastOrder));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/novel/series", params, headers));
}

json PixivClient::GetNovelDetails(PNID_t pnid)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"novel_id", pnid.to_string()}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/novel/detail", params, headers));
}

json PixivClient::GetNovelComments(PNID_t pnid, uint64_t offset, bool IncludeTotalComments)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"novel_id", pnid.to_string()}
	};
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));
	if (IncludeTotalComments)
		params.emplace("include_total_comments", "true");
	
	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/novel/comments", params, headers));
}

json PixivClient::GetNovelText(PNID_t pnid)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"novel_id", pnid.to_string()}
	};

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/novel/text", params, headers));
}

json PixivClient::SearchNovel(std::string keyword, SearchOption option, SortOrder sort,
			SearchDuration duration, std::string start_date, std::string end_state, 
			bool MergePlainKeywordResult, uint64_t offset)
{
	
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"search_target", to_string(option)},
		{"sort", to_string(sort)},
		{"filter", "for_android"}
	};
	params.emplace("word", std::move(keyword));
	if (MergePlainKeywordResult)
		params.emplace("merge_plain_keyword_results", "true");
	if (duration != SearchDuration::UNKNOWN)
		params.emplace("duration", to_string(duration));
	if (!start_date.empty())
		params.emplace("start_date", std::move(start_date));
	if (!end_state.empty())
		params.emplace("end_state", std::move(end_state));
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/search/novel", params, headers));
}

json PixivClient::GetRecommendedNovels(
		bool IncludeRankingNovels, bool IncludeRankingLabel, 
		PNID_t MaxBookmarkId,
		uint64_t offset, 
		const std::vector<PNID_t>& already_recommended,
		bool include_privacy_policy)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params{
		{"include_ranking_label", IncludeRankingLabel ? "true" : "false"},
		{"filter", "for_android"}
	};
	if (IncludeRankingNovels)
		params.emplace("include_ranking_novels", "true");
	if (MaxBookmarkId > 0_pid)
		params.emplace("max_bookmark_id_for_recommend", MaxBookmarkId.to_string());
	if (offset > 0)
		params.emplace("offset", std::to_string(offset));
	if (!already_recommended.empty())
	{
		string str = std::accumulate(already_recommended.begin(), already_recommended.end(), string{},
		[](const string& acc, PNID_t s){ return acc.empty()? s.to_string() : acc + "," + s.to_string();});
		params.emplace("already_recommended", str);
	}
	if (include_privacy_policy)
		params.emplace("include_privacy_policy", "true");

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v1/novel/recommended", params, headers));
}

json PixivClient::AutoComplete(string keyword, bool MergePlainKeywordResult)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	httplib::Params params;
	params.emplace("word", std::move(keyword));
	if (MergePlainKeywordResult)
		params.emplace("merge_plain_keyword_results", "true");

	return Utils::GetJsonResponse(this->_GetApiClient().Get("/v2/search/autocomplete", params, headers));
}

json PixivClient::NextPage(const std::string& NextUrl)
{
	auto token = this->_GetToken();
	auto headers = GetClientHashHeader();
	headers.emplace("Authorization", "Bearer " + token);
	
	Utils::UrlComponent component = Utils::UrlComponent::ParseUrl(NextUrl);

	return Utils::GetJsonResponse(this->_GetApiClient().Get(component.GetRelativeRef(), {}, headers));
}

}