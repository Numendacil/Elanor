#ifndef _PIXIV_CLIENT_HPP_
#define _PIXIV_CLIENT_HPP_

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <httplib.h>
#include <nlohmann/json_fwd.hpp>

#include "Models.hpp"

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
	httplib::Client _OauthCli;
	httplib::Client _ApiCli;
	httplib::Client _DownloadCli;

	const std::string _RefreshToken;
	const std::string _ProxyHost;
	const int _ProxyPort;

	std::string _AccessToken;
	std::chrono::system_clock::time_point _ObtainedTime;

	void _login();
	std::string _GetToken();

	mutable std::mutex _mtx;

	void _InitClients()
	{
		if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
		{
			this->_OauthCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
			this->_ApiCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
			this->_DownloadCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
		}

		// this->_OauthCli.set_keep_alive(true);
		// this->_ApiCli.set_keep_alive(true);
		// this->_DownloadCli.set_keep_alive(true);

		this->_OauthCli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
		this->_OauthCli.set_write_timeout(10); // NOLINT(*-avoid-magic-numbers)
		this->_OauthCli.set_read_timeout(60); // NOLINT(*-avoid-magic-numbers)

		this->_ApiCli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
		this->_ApiCli.set_write_timeout(10); // NOLINT(*-avoid-magic-numbers)
		this->_ApiCli.set_read_timeout(60); // NOLINT(*-avoid-magic-numbers)

		this->_DownloadCli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
		this->_DownloadCli.set_write_timeout(10); // NOLINT(*-avoid-magic-numbers)
		this->_DownloadCli.set_read_timeout(120); // NOLINT(*-avoid-magic-numbers)
	}

public:
	explicit PixivClient(std::string token, std::string proxy_host = {}, int proxy_port = -1) : 
	_RefreshToken(std::move(token)), _ProxyHost(std::move(proxy_host)), _ProxyPort(proxy_port),
	_OauthCli(oauth_hosts.data()), _ApiCli(api_hosts.data()), _DownloadCli(download_hosts.data())
	{
		this->_InitClients();
	}

	PixivClient(const PixivClient& rhs) :
	_RefreshToken(rhs._RefreshToken), _ProxyHost(rhs._ProxyHost), _ProxyPort(rhs._ProxyPort),
	_OauthCli(oauth_hosts.data()), _ApiCli(api_hosts.data()), _DownloadCli(download_hosts.data())
	{
		{
			std::lock_guard<std::mutex> lk(rhs._mtx);
			this->_AccessToken = rhs._AccessToken;
			this->_ObtainedTime = rhs._ObtainedTime;
		}

		this->_InitClients();
	}
	
	PixivClient(PixivClient&& rhs) :
	_RefreshToken(rhs._RefreshToken), _ProxyHost(rhs._ProxyHost), _ProxyPort(rhs._ProxyPort),
	_OauthCli(oauth_hosts.data()), _ApiCli(api_hosts.data()), _DownloadCli(download_hosts.data())
	{
		{
			std::lock_guard<std::mutex> lk(rhs._mtx);
			this->_AccessToken = std::move(rhs._AccessToken);
			this->_ObtainedTime = std::move(rhs._ObtainedTime);
		}

		this->_InitClients();
	}

	PixivClient& operator=(const PixivClient&) = delete;
	PixivClient& operator=(PixivClient&&) = delete;
	~PixivClient() = default;

	std::string DownloadIllust(const std::string& url);
	void DownloadIllust(const std::string& url, httplib::ContentReceiver receiver);

	using json = nlohmann::json;

	// 用户详情
	json GetUserDetails(PUID_t UserId);

	// 搜索用户
	json SearchUser(std::string name, uint64_t offset = 0);

	// 关注用户
	json FollowUser(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC);

	// 取消关注用户
	json UnfollowUser(PUID_t UserId);

	// 用户收藏标签
	json GetUserBookmarkTags(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, uint64_t offset = 0);
	
	// 用户作品
	json GetUserIllusts(PUID_t UserId, ContentType type = ContentType::ILLUST, uint64_t offset = 0);

	// 用户小说列表
	json GetUserNovels(PUID_t UserId, uint64_t offset = 0);

	// 用户关注列表
	json GetUserFollowing(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, uint64_t offset = 0);

	// 用户被关注列表
	json GetUserFollower(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, uint64_t offset = 0);

	// 用户好P友列表
	json GetUserMyPixiv(PUID_t UserId, uint64_t offset = 0);

	// 用户黑名单
	json GetUserBlacklist(PUID_t UserId, uint64_t offset = 0);

	// 用户收藏作品
	json GetUserBookmarkedIllusts(PUID_t UserId, RESTRICT restrict = RESTRICT::PUBLIC, 
				PID_t MaxBookmarkId = 0_pid, std::string tag = {});

	// 相关用户推荐
	json GetRelatedUsers(PUID_t UserId, uint64_t offset = 0);

	// 推荐用户
	json GetRecommendedUser(uint64_t offset = 0);

	// 已关注用户新作
	json GetFollowedIllust(RESTRICT restrict = RESTRICT::ALL, uint64_t offset = 0);

	// 作品详情
	json GetIllustDetails(PID_t pid);

	// 作品评论
	json GetIllustComments(PID_t pid, uint64_t offset = 0, bool IncludeTotalComments = true);

	// 相关作品推荐
	json GetRelatedIllusts(PID_t pid, uint64_t offset = 0, const std::vector<PID_t>& seeds = {});

	// 主页插画推荐
	json GetRecommendedIllusts(ContentType type = ContentType::ILLUST, 
		bool IncludeRankingIllusts = false, bool IncludeRankingLabel = true, 
		PID_t MinBookmarkId = 0_pid, PID_t MaxBookmarkId = 0_pid,
		uint64_t offset = 0, 
		const std::vector<PID_t>& bookmark_illust_ids = {},
		const std::vector<PID_t>& viewed_ids = {},
		bool include_privacy_policy = true);	// type only accepts ILLUST and MANGA

	// 作品排行
	json GetIllustRanking(RankingMode option = RankingMode::DAY, std::string date = {},	// Date format %Y-%m-%d
		uint64_t offset = 0);

	// 最新最热作画标签
	json GetTrendingTagsIllust();

	// 最新最热小说标签
	json GetTrendingTagsNovel();

	// 搜索插画
	json SearchIllust(std::string keyword, SearchOption option = SearchOption::TAGS_PARTIAL, SortOrder sort = SortOrder::DATE_DESC,
			SearchDuration duration = SearchDuration::UNKNOWN, std::string start_date = {}, std::string end_state = {}, 
			bool MergePlainKeywordResult = true, uint64_t offset = 0);

	// 作品收藏详情
	json GetIllustBookmarkDetails(PID_t pid);

	// 收藏作品
	json AddBookmark(PID_t pid, RESTRICT restrict = RESTRICT::PUBLIC, const std::vector<std::string>& tags = {});

	// 取消收藏作品
	json DeleteBookmark(PID_t pid);

	// 获取动图信息
	json GetUgoiraMetadata(PID_t pid);

	// 小说系列详情
	json GetNovelSeries(uint64_t SeriesId, std::string LastOrder = {});

	// 小说详情
	json GetNovelDetails(PNID_t pnid);

	// 作品评论
	json GetNovelComments(PNID_t pnid, uint64_t offset = 0, bool IncludeTotalComments = true);

	// 小说正文
	json GetNovelText(PNID_t pnid);

	// 搜索小说
	json SearchNovel(std::string keyword, SearchOption option = SearchOption::TAGS_PARTIAL, SortOrder sort = SortOrder::DATE_DESC,
			SearchDuration duration = SearchDuration::UNKNOWN, std::string start_date = {}, std::string end_state = {}, 
			bool MergePlainKeywordResult = true, uint64_t offset = 0);

	// 小说推荐
	json GetRecommendedNovels(
		bool IncludeRankingNovels = false, bool IncludeRankingLabel = true, 
		PNID_t MaxBookmarkId = 0_pnid,
		uint64_t offset = 0, 
		const std::vector<PNID_t>& already_recommended = {},
		bool include_privacy_policy = true);

	// 关键词联想
	json AutoComplete(std::string keyword, bool MergePlainKeywordResult = true);

	// 下一页内容
	json NextPage(const std::string& NextUrl);
};

}

#endif