#ifndef _SEKAI_NETWORKDCLIENT_HPP_
#define _SEKAI_NETWORKDCLIENT_HPP_


#include <chrono>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <httplib.h>
#include <nlohmann/json_fwd.hpp>

#include <models/BasicTypes.hpp>

namespace Sekai
{

class SekaiNetworkClient
{
protected:
	httplib::Client _cli;

	const std::string _AESKey;
	const std::string _AESIV;

	const std::string _ProxyHost;
	const int _ProxyPort;

	std::string _AppVersion;
	std::string _AppHash;
	std::string _AssetVersion;
	std::string _AssetHash;
	std::string _DataVersion;
	std::string _AssetBundleHostHash;

	mutable std::mutex _mtx;

	/* Utilities */

	// std::string _encode(std::string content) const;
	// std::string _decode(std::string content) const;

	std::string _GetRequestID() const;
	httplib::Headers _GetHeader();

	void _VerifyAndThrow(const httplib::Result& result) const;

	/* Token Related */

	static constexpr auto TOKEN_TIMEOUT = std::chrono::hours(1) * 0.95;
	struct Token
	{
		Account user;
		std::string SessionToken;

		std::chrono::system_clock::time_point LastUsed;
		bool inUse = false;
	};

	std::map<UID_t, Token> _TokenPool;

	// Return {user_id, session_token} pair
	std::pair<UID_t, std::string> _GetToken();
	// If api request failed, return empty token and timestamp will not be updated
	void _RefreshToken(UID_t user, std::string token = {});

	/* Cookie Related */

	static constexpr auto COOKIE_TIMEOUT = std::chrono::days(1) * 0.95;
	struct Cookie
	{
		std::string cookie;
		std::chrono::system_clock::time_point LastUsed;
	} _cookie;

	std::string _GetCookie();

public:
	explicit SekaiNetworkClient(
		std::string key, std::string iv, 
		const std::string& ApiUrl,
		std::string proxy_host = {}, int proxy_port = -1,
		std::string AppVersion = "2.3.5",
		std::string AppHash = "cc22bebb-bce8-1744-2543-16a166dd220d",
		std::string AssetVersion = "2.3.5.10",
		std::string AssetHash = "59b61c36-1d09-b6d7-1921-233766dce4c1",
		std::string DataVersion = "2.3.5.11",
		std::string AssetBundleHostHash = "cf2d2388"
	);

	void AddCredentials(const std::vector<Account>& users);	
	std::vector<Account> GetCredentials() const;	

	nlohmann::json GetVersionInfo();
	nlohmann::json GetGameVersionInfo();
	nlohmann::json Login(const Account& user);
	nlohmann::json Register();

	nlohmann::json GetUserProfile(UID_t UserId);
	nlohmann::json GetUserEventRanking(UID_t UserId, uint64_t EventId);
	// LowerLimit must be smaller than 100
	nlohmann::json GetEventRankingUser(uint64_t TargetRank, uint64_t EventId, uint16_t LowerLimit = 1);

	nlohmann::json GetAssetList();

	// Too large to decode to json
	std::string GetMasterSuiteInfo();
};

}

#endif