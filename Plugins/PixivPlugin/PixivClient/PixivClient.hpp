#ifndef _PIXIV_CLIENT_HPP_
#define _PIXIV_CLIENT_HPP_

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "Models.hpp"

namespace httplib
{

class Client;

}

namespace Pixiv
{

class PixivClient
{
	static constexpr std::string_view client_id = "MOBrBDS8blbauoSck0ZfDbtuzpyT";
	static constexpr std::string_view client_secret = "lsACyCD94FhDUtGTXi3QzcFE2uU1hqtDaKeqrdwj";

	static constexpr std::string_view api_hosts = "https://app-api.pixiv.net";
	static constexpr std::string_view oauth_hosts = "https://oauth.secure.pixiv.net";
	static constexpr std::string_view download_hosts = "https://i.pximg.net";

	static constexpr auto expire_timeout = std::chrono::seconds(3600) * 0.95;

protected:
	const std::string _RefreshToken;
	const std::string _ProxyHost;
	const int _ProxyPort;

	std::string _AccessToken;
	std::chrono::system_clock::time_point _ObtainedTime;

	void _login();
	std::string _GetToken();

	mutable std::mutex _mtx;

	// void _InitClients()
	// {
	// 	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
	// 	{
	// 		this->_OauthCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
	// 		this->_ApiCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
	// 		this->_DownloadCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
	// 	}

	// 	// this->_OauthCli.set_keep_alive(true);
	// 	// this->_ApiCli.set_keep_alive(true);
	// 	// this->_DownloadCli.set_keep_alive(true);

	// 	this->_OauthCli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	// 	this->_OauthCli.set_write_timeout(10); // NOLINT(*-avoid-magic-numbers)
	// 	this->_OauthCli.set_read_timeout(60); // NOLINT(*-avoid-magic-numbers)

	// 	this->_ApiCli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	// 	this->_ApiCli.set_write_timeout(10); // NOLINT(*-avoid-magic-numbers)
	// 	this->_ApiCli.set_read_timeout(60); // NOLINT(*-avoid-magic-numbers)

	// 	this->_DownloadCli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	// 	this->_DownloadCli.set_write_timeout(10); // NOLINT(*-avoid-magic-numbers)
	// 	this->_DownloadCli.set_read_timeout(120); // NOLINT(*-avoid-magic-numbers)
	// }

	httplib::Client _GetOAuthClient() const;
	httplib::Client _GetApiClient() const;
	httplib::Client _GetDownloadClient() const;

public:
	explicit PixivClient(std::string token, std::string ProxyHost = {}, int ProxyPort = -1) : 
	_RefreshToken(std::move(token)), _ProxyHost(std::move(ProxyHost)), _ProxyPort(ProxyPort)
	{
	}

	PixivClient(const PixivClient& rhs) :
	_RefreshToken(rhs._RefreshToken), _ProxyHost(rhs._ProxyHost), _ProxyPort(rhs._ProxyPort)
	{
		{
		
		std::lock_guard<std::mutex> lk(rhs._mtx);
		this->_AccessToken = rhs._AccessToken;
		this->_ObtainedTime = rhs._ObtainedTime;
	
		}
	}
	
	PixivClient(PixivClient&& rhs) :
	_RefreshToken(rhs._RefreshToken), _ProxyHost(rhs._ProxyHost), _ProxyPort(rhs._ProxyPort)
	{
		{

		std::lock_guard<std::mutex> lk(rhs._mtx);
		this->_AccessToken = std::move(rhs._AccessToken);
		this->_ObtainedTime = std::move(rhs._ObtainedTime);
		
		}
	}

	PixivClient& operator=(const PixivClient& rhs) = delete;
	PixivClient& operator=(PixivClient&&) = delete;
	~PixivClient() = default;

	// ????????????
	std::string DownloadIllust(const std::string& url);
	void DownloadIllust(const std::string& url, std::function<bool(const char*, size_t)> receiver);
	std::vector<std::string> BatchDownloadIllust(const std::vector<std::string>& urls);
	void BatchDownloadIllust(const std::vector<std::string>& urls, std::function<bool(std::string, size_t)> receiver);

	using json = nlohmann::json;

	// ????????????
	json GetUserDetails(PUID_t UserId);

	// ????????????
	json SearchUser(std::string name, uint64_t offset = 0);

	// ????????????
	json FollowUser(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC);

	// ??????????????????
	json UnfollowUser(PUID_t UserId);

	// ??????????????????
	json GetUserBookmarkTags(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, uint64_t offset = 0);
	
	// ????????????
	json GetUserIllusts(PUID_t UserId, ContentType type = ContentType::ILLUST, uint64_t offset = 0);

	// ??????????????????
	json GetUserNovels(PUID_t UserId, uint64_t offset = 0);

	// ??????????????????
	json GetUserFollowing(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, uint64_t offset = 0);

	// ?????????????????????
	json GetUserFollower(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, uint64_t offset = 0);

	// ?????????P?????????
	json GetUserMyPixiv(PUID_t UserId, uint64_t offset = 0);

	// ???????????????
	json GetUserBlacklist(PUID_t UserId, uint64_t offset = 0);

	// ??????????????????
	json GetUserBookmarkedIllusts(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, 
				PID_t MaxBookmarkId = 0_pid, std::string tag = {});

	// ??????????????????
	json GetRelatedUsers(PUID_t UserId, uint64_t offset = 0);

	// ????????????
	json GetRecommendedUser(uint64_t offset = 0);

	// ?????????????????????
	json GetFollowedIllust(RESTRICT restrict = RESTRICT::ALL, uint64_t offset = 0);

	// ????????????
	json GetIllustDetails(PID_t pid);

	// ????????????
	json GetIllustComments(PID_t pid, uint64_t offset = 0, bool IncludeTotalComments = true);

	// ??????????????????
	json GetRelatedIllusts(PID_t pid, uint64_t offset = 0, const std::vector<PID_t>& seeds = {});

	// ??????????????????
	json GetRecommendedIllusts(ContentType type = ContentType::ILLUST, 
		bool IncludeRankingIllusts = false, bool IncludeRankingLabel = true, 
		PID_t MinBookmarkId = 0_pid, PID_t MaxBookmarkId = 0_pid,
		uint64_t offset = 0, 
		const std::vector<PID_t>& bookmark_illust_ids = {},
		const std::vector<PID_t>& viewed_ids = {},
		bool include_privacy_policy = true);	// type only accepts ILLUST and MANGA

	// ????????????
	json GetIllustRanking(RankingMode option = RankingMode::DAY, std::string date = {},	// Date format %Y-%m-%d
		uint64_t offset = 0);

	// ????????????????????????
	json GetTrendingTagsIllust();

	// ????????????????????????
	json GetTrendingTagsNovel();

	// ????????????
	json SearchIllust(std::string keyword, SearchOption option = SearchOption::TAGS_PARTIAL, SortOrder sort = SortOrder::DATE_DESC,
			SearchDuration duration = SearchDuration::UNKNOWN, std::string start_date = {}, std::string end_state = {}, 
			bool MergePlainKeywordResult = true, uint64_t offset = 0);

	// ??????????????????
	json GetIllustBookmarkDetails(PID_t pid);

	// ????????????
	json AddBookmark(PID_t pid, RESTRICT restrict = RESTRICT::PUBLIC, const std::vector<std::string>& tags = {});

	// ??????????????????
	json DeleteBookmark(PID_t pid);

	// ??????????????????
	json GetUgoiraMetadata(PID_t pid);

	// ??????????????????
	json GetNovelSeries(uint64_t SeriesId, std::string LastOrder = {});

	// ????????????
	json GetNovelDetails(PNID_t pnid);

	// ????????????
	json GetNovelComments(PNID_t pnid, uint64_t offset = 0, bool IncludeTotalComments = true);

	// ????????????
	json GetNovelText(PNID_t pnid);

	// ????????????
	json SearchNovel(std::string keyword, SearchOption option = SearchOption::TAGS_PARTIAL, SortOrder sort = SortOrder::DATE_DESC,
			SearchDuration duration = SearchDuration::UNKNOWN, std::string start_date = {}, std::string end_state = {}, 
			bool MergePlainKeywordResult = true, uint64_t offset = 0);

	// ????????????
	json GetRecommendedNovels(
		bool IncludeRankingNovels = false, bool IncludeRankingLabel = true, 
		PNID_t MaxBookmarkId = 0_pnid,
		uint64_t offset = 0, 
		const std::vector<PNID_t>& already_recommended = {},
		bool include_privacy_policy = true);

	// ???????????????
	json AutoComplete(std::string keyword, bool MergePlainKeywordResult = true);

	// ???????????????
	json NextPage(const std::string& NextUrl);
};

}

#endif