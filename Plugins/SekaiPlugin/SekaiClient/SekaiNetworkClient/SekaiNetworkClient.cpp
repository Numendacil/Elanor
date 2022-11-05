#include "SekaiNetworkClient.hpp"

#include <cassert>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <openssl/evp.h>

#include <nlohmann/json.hpp>
#include <httplib.h>
#include <stduuid/include/uuid.h>

#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>

#include <PluginUtils/NetworkUtils.hpp>

#include <models/Exceptions.hpp>
#include <models/BasicTypes.hpp>

using std::string;
using json = nlohmann::json;
namespace chrono = std::chrono;

namespace Sekai
{

namespace
{


std::string EncodeAES(std::string content, std::string_view AESKey, std::string_view AESIV)
{
	if (content.empty()) return {};

	std::unique_ptr<EVP_CIPHER_CTX, std::function<void(EVP_CIPHER_CTX*)>> ctx(
		EVP_CIPHER_CTX_new(),
		EVP_CIPHER_CTX_free
	);
	if (!ctx)
		throw AESError("Failed to create cipher context");
	
	const auto* key = reinterpret_cast<const unsigned char*>(AESKey.data());
	const auto* iv = reinterpret_cast<const unsigned char*>(AESIV.data());

	int rc = EVP_EncryptInit_ex(
		ctx.get(), 
		EVP_aes_128_cbc(), 
		nullptr, 
		key, 
		iv
	);
	if(rc != 1)
		throw AESError("Failed to init encrypt");

	constexpr size_t BLOCK_SIZE = 16;
	std::string encrypted_str(content.size() + BLOCK_SIZE, 0);
	int out_len = static_cast<int>(encrypted_str.size());
	rc = EVP_EncryptUpdate(
		ctx.get(), 
		reinterpret_cast<unsigned char*>(encrypted_str.data()), 
		&out_len, 
		reinterpret_cast<const unsigned char*>(content.c_str()), 
		static_cast<int>(content.size())
	);
	if (rc != 1)
		throw AESError("EVP_EncryptUpdate failed");

	int len = static_cast<int>(encrypted_str.size()) - out_len;
	rc = EVP_EncryptFinal_ex(
		ctx.get(),
		reinterpret_cast<unsigned char*>(encrypted_str.data()) + out_len,
		&len
	);
	if (rc != 1)
		throw AESError("EVP_EncryptFinal_ex failed");
	encrypted_str.resize(out_len + len);
	return encrypted_str;
}

std::string DecodeAES(std::string content, std::string_view AESKey, std::string_view AESIV)
{
	if (content.empty()) return {};

	std::unique_ptr<EVP_CIPHER_CTX, std::function<void(EVP_CIPHER_CTX*)>> ctx(
		EVP_CIPHER_CTX_new(),
		EVP_CIPHER_CTX_free
	);
	if (!ctx)
		throw AESError("Failed to create cipher context");
	
	const auto* key = reinterpret_cast<const unsigned char*>(AESKey.data());
	const auto* iv = reinterpret_cast<const unsigned char*>(AESIV.data());

	int rc = EVP_DecryptInit_ex(
		ctx.get(), 
		EVP_aes_128_cbc(), 
		nullptr, 
		key, 
		iv
	);
	if(rc != 1)
		throw AESError("Failed to init decrypt");

	std::string decrypted_str(content.size(), 0);
	int out_len = static_cast<int>(decrypted_str.size());
	rc = EVP_DecryptUpdate(
		ctx.get(), 
		reinterpret_cast<unsigned char*>(decrypted_str.data()), 
		&out_len, 
		reinterpret_cast<const unsigned char*>(content.c_str()), 
		static_cast<int>(content.size())
	);
	if (rc != 1)
		throw AESError("EVP_DecryptUpdate failed");

	int len = static_cast<int>(decrypted_str.size()) - out_len;
	rc = EVP_DecryptFinal_ex(
		ctx.get(),
		reinterpret_cast<unsigned char*>(decrypted_str.data()) + out_len,
		&len
	);
	if (rc != 1)
		throw AESError("EVP_DecryptFinal_ex failed");
	decrypted_str.resize(out_len + len);
	return decrypted_str;
}

constexpr auto X_INSTALL_ID = "91b6f2f8-16d2-4541-aaf9-d7166eafcbb9";
constexpr auto X_PLATFORM = "iOS";
constexpr auto X_DEVICE_MODEL = "iPad6,3";
constexpr auto X_OPERATING_SYSTEM = "iPadOS 15.6.1";
constexpr auto X_KC = "fba86091-8ce8-4ee5-a4e9-0ba1c63329bb";
constexpr auto X_AI = "2e7425d19436a4845e8ab2e6fe9d4a6c";
constexpr auto X_UNITY_VERSION = "2020.3.32f1";

}



SekaiNetworkClient::SekaiNetworkClient(
	string key, string iv, 
	string proxy_host, int proxy_port,
	string AppVersion, 
	string AppHash, 
	string AssetVersion, 
	string AssetHash, 
	string DataVersion,
	string AssetBundleHostHash
) 
: _ApiCli("https://production-game-api.sekai.colorfulpalette.org"),
  _CookieCli("https://issue.sekai.colorfulpalette.org"),
  _GameVersionCli("https://sekai-proxy.workers.thanatos.xyz"),		// reverse proxy by cloudflare workers
  _AESKey(std::move(key)), _AESIV(std::move(iv)),
  _ProxyHost(std::move(proxy_host)), _ProxyPort(proxy_port),
  _AppVersion(std::move(AppVersion)), 
  _AppHash(std::move(AppHash)), 
  _AssetVersion(std::move(AssetVersion)),
  _AssetHash(std::move(AssetHash)),
  _DataVersion(std::move(DataVersion)),
  _AssetBundleHostHash(std::move(AssetBundleHostHash))
{
	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
	{
		this->_ApiCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
		this->_CookieCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
		this->_GameVersionCli.set_proxy(this->_ProxyHost, this->_ProxyPort);
	}
	else
	{
		this->_ApiCli.set_keep_alive(true);
	}

	this->_ApiCli.set_connection_timeout(60); // NOLINT(*-avoid-magic-numbers)
	this->_ApiCli.set_write_timeout(60); // NOLINT(*-avoid-magic-numbers)
	this->_ApiCli.set_read_timeout(300); // NOLINT(*-avoid-magic-numbers)

	this->_ApiCli.set_default_headers({
		// {"Host", "production-game-api.sekai.colorfulpalette.org"},
		{"Content-Type", "application/octet-stream"},
		{"Accept", "application/octet-stream"},
		{"Accept-Encoding", "deflate, gzip"},
		{"Accept-Language", "en-US,en;q=0.9"},
		{"User-Agent", "ProductName/94 CFNetwork/1335.0.3 Darwin/21.6.0"},
		{"X-Install-Id", X_INSTALL_ID},
		{"X-Platform", X_PLATFORM},
		{"X-DeviceModel", X_DEVICE_MODEL},
		{"X-OperatingSystem", X_OPERATING_SYSTEM},
		{"X-KC", X_KC},
		{"X-Unity-Version", X_UNITY_VERSION},
		{"X-AI", X_AI}
	});

	this->_CookieCli.set_connection_timeout(60);		// NOLINT(*-avoid-magic-numbers)
	this->_CookieCli.set_write_timeout(60);		// NOLINT(*-avoid-magic-numbers)
	this->_CookieCli.set_read_timeout(300);		// NOLINT(*-avoid-magic-numbers)

	this->_CookieCli.set_default_headers({
		// {"Host", "issue.sekai.colorfulpalette.org"},
		{"Accept", "*/*"},
		{"Accept-Encoding", "deflate, gzip"},
		{"Accept-Language", "en-US,en;q=0.9"},
		{"User-Agent", "ProductName/94 CFNetwork/1335.0.3 Darwin/21.6.0"}
	});

	this->_GameVersionCli.set_connection_timeout(60);		// NOLINT(*-avoid-magic-numbers)
	this->_GameVersionCli.set_write_timeout(60);		// NOLINT(*-avoid-magic-numbers)
	this->_GameVersionCli.set_read_timeout(300);		// NOLINT(*-avoid-magic-numbers)

	this->_GameVersionCli.set_default_headers({
		// {"Host", "game-version.sekai.colorfulpalette.org"},
		{"Content-Type", "application/octet-stream"},
		{"Accept", "application/octet-stream"},
		{"Accept-Encoding", "deflate, gzip"},
		{"Accept-Language", "en-US,en;q=0.9"},
		{"User-Agent", "ProductName/94 CFNetwork/1335.0.3 Darwin/21.6.0"},
		{"X-Install-Id", X_INSTALL_ID},
		{"X-Platform", X_PLATFORM},
		{"X-DeviceModel", X_DEVICE_MODEL},
		{"X-OperatingSystem", X_OPERATING_SYSTEM},
		{"X-KC", X_KC},
		{"X-Unity-Version", X_UNITY_VERSION},
		{"X-AI", X_AI}
	});
}


std::string SekaiNetworkClient::_GetRequestID() const
{
	uuids::basic_uuid_random_generator rng(Utils::GetRngEngine());
	return uuids::to_string(rng());
}

void SekaiNetworkClient::_VerifyAndThrow(const httplib::Result& result) const
{
	if (Utils::VerifyResponse(result))
		return;

	if (!result)
		throw NetworkException(httplib::to_string(result.error()), "", -1);

	switch (result->status)
	{
	case 426:	// NOLINT(*-avoid-magic-numbers)
		throw UpgradeRequired();
	default:
		if (result->body.empty())
			throw NetworkException(result->reason, "", result->status);
		else
		{
			std::string msg = result->body;
			try
			{
				msg = DecodeAES(msg, this->_AESKey, this->_AESIV);
				msg = json::from_msgpack(msg).dump();
			}
			catch(...) {}
			throw NetworkException(result->reason, msg, result->status);
		}
	}
}

httplib::Headers SekaiNetworkClient::_GetHeader()
{
	string AppVersion;
	string AssetVersion;
	string DataVersion;

	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		AppVersion = this->_AppVersion;
		AssetVersion = this->_AssetVersion;
		DataVersion = this->_DataVersion;
	}

	return {
		{"X-App-Version", std::move(AppVersion)},
		{"X-Asset-Version", std::move(AssetVersion)},
		{"X-Data-Version", std::move(DataVersion)},
		{"X-Request-Id", this->_GetRequestID()},
		{"Cookie", this->_GetCookie()}
	};
}



void SekaiNetworkClient::AddCredentials(const std::vector<Account>& users)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	for (const auto& user : users)
	{
		Token tok;
		tok.user = user;
		this->_TokenPool.emplace(user.id, std::move(tok));
	}
}

std::vector<Account> SekaiNetworkClient::GetCredentials() const
{
	std::vector<Account> users;

	std::lock_guard<std::mutex> lk(this->_mtx);
	users.reserve(this->_TokenPool.size());
	for (const auto& token : this->_TokenPool)
		users.push_back(token.second.user);
	return users;
}

std::pair<UID_t, string> SekaiNetworkClient::_GetToken()
{
	Token token;
	bool hasToken = false;
	bool token_valid = false;

	while (true)
	{
		if (!hasToken)
		{
			// Search cache for available tokens

			bool found = false;
			{
				std::lock_guard<std::mutex> lk(this->_mtx);
				for (auto& [id, tok] : this->_TokenPool)
				{
					if (!tok.inUse)
					{
						token = tok;
						found = true;
					
						tok.inUse = true;
						tok.LastUsed = chrono::system_clock::now();
						break;
					}
					else
					{
						// In case someone forgot to return back
						if (chrono::system_clock::now() - token.LastUsed > TOKEN_TIMEOUT * 2)
						{
							tok.inUse = false;
						}
					}
				}
			}
			
			if (found)
			{
				LOG_DEBUG(Utils::GetLogger(), "Token found from pool");
				token_valid = !token.SessionToken.empty() &&
						chrono::system_clock::now() - token.LastUsed < TOKEN_TIMEOUT;
			}
			else
			{
				LOG_DEBUG(Utils::GetLogger(), "Token not found from pool, creating a new one");
				// Register a new one;
				json result = this->Register();
				token.user.id = result.at("userRegistration").at("userId").get<UID_t>();
				token.user.credential = result.at("credential").get<std::string>();
				token.inUse = true;
				token.LastUsed = chrono::system_clock::now();
				{
					std::lock_guard<std::mutex> lk(this->_mtx);
					this->_TokenPool.emplace(token.user.id, token);
				}
				token_valid = false;
			}
			hasToken = true;
		}

		if (!token_valid)
		{
			// Renew token by auth

			try
			{
				LOG_INFO(Utils::GetLogger(), "Session invalid, creating new session");
				json result = this->Login(token.user);
				token.SessionToken = result.at("sessionToken").get<string>();
				{
					std::lock_guard<std::mutex> lk(this->_mtx);
					this->_AppVersion = result.at("appVersion").get<string>();
					this->_AssetVersion = result.at("assetVersion").get<string>();
					this->_AssetHash = result.at("assetHash").get<string>();
					this->_DataVersion = result.at("dataVersion").get<string>();
					this->_TokenPool.at(token.user.id).SessionToken = token.SessionToken;
				}
				return {token.user.id, token.SessionToken};
			}
			catch(const UpgradeRequired& e)
			{
				LOG_WARN(Utils::GetLogger(), "Version outdated, get new version info");
				(void)this->GetVersionInfo();
				continue;
			}
			catch(const NetworkException& e)
			{
				// NOLINTNEXTLINE(*-avoid-magic-numbers)
				if (e._code == 403 || e._code == 400 || e._code == 500)
				{
					LOG_WARN(Utils::GetLogger(), "Invalid credential, removing");
					hasToken = false;
					token = Token{};
					{
						std::lock_guard<std::mutex> lk(this->_mtx);
						this->_TokenPool.erase(token.user.id);
					}
					continue;
				}
				else
					throw;
			}
		}
		else
		{
			return {token.user.id, token.SessionToken};
		}

	}

}

void SekaiNetworkClient::_RefreshToken(UID_t user, string token)
{
	if (token.empty())
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_TokenPool.at(user).inUse = false;
	}
	else
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto& tok = this->_TokenPool.at(user);
		tok.inUse = false;
		tok.LastUsed = chrono::system_clock::now();
		tok.SessionToken = std::move(token);
	}
}



string SekaiNetworkClient::_GetCookie()
{
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		if (!this->_cookie.cookie.empty() &&
			chrono::system_clock::now() - this->_cookie.LastUsed < COOKIE_TIMEOUT)
			return this->_cookie.cookie;
	}

	LOG_DEBUG(Utils::GetLogger(), "Cookie expired, applying for a new one");

	auto result = this->_CookieCli.Post("/api/signature");
	this->_VerifyAndThrow(result);

	if (result->has_header("Set-Cookie"))
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_cookie.cookie = result->get_header_value("Set-Cookie");
		this->_cookie.LastUsed = chrono::system_clock::now();
		LOG_DEBUG(Utils::GetLogger(), "New cookie: " + this->_cookie.cookie);
		return this->_cookie.cookie;
	}
	else
		throw CookieError("Response header does not contain 'Set-Cookie': \n"
		+ [&result]() -> string 
		{
			string header;
			for (const auto [key, value] : result->headers)
				header += key + ": " + value + "\n";
			return header;
		}());
}



json SekaiNetworkClient::GetVersionInfo()
{
	auto result = this->_ApiCli.Get("/api/system", this->_GetHeader());
	this->_VerifyAndThrow(result);
	json versions = json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));

	for (const auto& version : versions.at("appVersions"))
	{
		if (version.at("appVersionStatus").get<string>() == "available" && 
		version.at("systemProfile").get<string>() == "production")
		{
			std::lock_guard<std::mutex> lk(this->_mtx);
			this->_AppVersion = version.at("appVersion").get<string>();
			this->_AppHash = version.at("appHash").get<string>();
			this->_AssetVersion = version.at("assetVersion").get<string>();
			this->_AssetHash = version.at("assetHash").get<string>();
			this->_DataVersion = version.at("dataVersion").get<string>();
		}
	}

	return versions;
}

json SekaiNetworkClient::GetGameVersionInfo()
{
	string AppVersion;
	string AppHash;
	string AssetVersion;
	string DataVersion;
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		AppVersion = this->_AppVersion;
		AppHash = this->_AppHash;
		AssetVersion = this->_AssetVersion;
		DataVersion = this->_DataVersion;
	}

	auto result = this->_GameVersionCli.Get("/" + AppVersion + "/" + AppHash, {
		{"X-App-Version", AppVersion},
		{"X-Asset-Version", AssetVersion},
		{"X-Data-Version", DataVersion},
		{"X-Request-Id", this->_GetRequestID()},
		{"Cookie", this->_GetCookie()}
	});

	this->_VerifyAndThrow(result);
	json resp = json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		resp.at("assetbundleHostHash").get_to(this->_AssetBundleHostHash);
	}

	return resp;
}

json SekaiNetworkClient::Login(const Account& user)
{
	LOG_INFO(Utils::GetLogger(), "Calling login");
	auto data = json::to_msgpack({
		{"credential", user.credential}
	});
	auto result = this->_ApiCli.Put(
		"/api/user/" + user.id.to_string() + "/auth", 
		this->_GetHeader(),
		EncodeAES({data.begin(), data.end()}, this->_AESKey, this->_AESIV), 
		{}
	);
	this->_VerifyAndThrow(result);
	return json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));
}

json SekaiNetworkClient::Register()
{
	LOG_INFO(Utils::GetLogger(), "Calling register");
	auto data = json::to_msgpack({
		{"platform", X_PLATFORM},
		{"deviceModel", X_DEVICE_MODEL},
		{"operatingSystem", X_OPERATING_SYSTEM}
	});
	auto result = this->_ApiCli.Post(
		"/api/user", 
		this->_GetHeader(),
		EncodeAES({data.begin(), data.end()}, this->_AESKey, this->_AESIV), 
		{}
	);
	this->_VerifyAndThrow(result);
	return json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));
}



json SekaiNetworkClient::GetUserProfile(UID_t UserId)
{
	auto [id, token] = this->_GetToken();
	auto header = this->_GetHeader();
	header.emplace("X-Session-Token", std::move(token));

	auto result = this->_ApiCli.Get("/api/user/" + UserId.to_string() + "/profile", std::move(header));

	if (result && result->has_header("X-Session-Token"))
		this->_RefreshToken(id, result->get_header_value("X-Session-Token"));
	else
		this->_RefreshToken(id);
	
	this->_VerifyAndThrow(result);
	return json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));
}

json SekaiNetworkClient::GetUserEventRanking(UID_t UserId, uint64_t EventId)
{
	auto [id, token] = this->_GetToken();
	auto header = this->_GetHeader();
	header.emplace("X-Session-Token", std::move(token));

	auto result = this->_ApiCli.Get(
		"/api/user/" + id.to_string() + 
		"/event/" + std::to_string(EventId) + 
		"/ranking?targetUserId=" + UserId.to_string(), 
		std::move(header));
	
	if (result && result->has_header("X-Session-Token"))
		this->_RefreshToken(id, result->get_header_value("X-Session-Token"));
	else
		this->_RefreshToken(id);
	
	this->_VerifyAndThrow(result);
	return json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));
}

json SekaiNetworkClient::GetEventRankingUser(uint64_t TargetRank, uint64_t EventId, uint16_t LowerLimit)
{
	auto [id, token] = this->_GetToken();
	auto header = this->_GetHeader();
	header.emplace("X-Session-Token", std::move(token));

	std::string url = "/api/user/" + id.to_string() + 
		"/event/" + std::to_string(EventId) + 
		"/ranking?targetRank=" + std::to_string(TargetRank);

	if (LowerLimit > 1)
		url += "&lowerLimit=" + std::to_string(LowerLimit);

	auto result = this->_ApiCli.Get(url, std::move(header));
	
	if (result && result->has_header("X-Session-Token"))
		this->_RefreshToken(id, result->get_header_value("X-Session-Token"));
	else
		this->_RefreshToken(id);
	
	this->_VerifyAndThrow(result);
	return json::from_msgpack(DecodeAES(result->body, this->_AESKey, this->_AESIV));
}


namespace
{

struct AESContentReceiver
{
	std::string content;
	std::unique_ptr<EVP_CIPHER_CTX, std::function<void(EVP_CIPHER_CTX*)>> ctx;

	AESContentReceiver(std::string_view AESKey, std::string_view AESIV) : 
	ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free)
	{
		if (!ctx)
			throw AESError("Failed to create cipher context");
		
		const auto* key = reinterpret_cast<const unsigned char*>(AESKey.data());
		const auto* iv = reinterpret_cast<const unsigned char*>(AESIV.data());

		int rc = EVP_DecryptInit_ex(
			ctx.get(), 
			EVP_aes_128_cbc(), 
			nullptr, 
			key, 
			iv
		);
		if(rc != 1)
			throw AESError("Failed to init decrypt");
		this->content.reserve(1024 * 1024);	// NOLINT(*-avoid-magic-numbers)
	}

	bool Receiver(const char* data, size_t n)
	{
		constexpr size_t BLOCK_SIZE = 16;
		constexpr size_t BUFFER_SIZE = 4096;
		char buffer[BUFFER_SIZE + BLOCK_SIZE];	// NOLINT(*-avoid-c-arrays)

		int out_len = sizeof(buffer);
		while (n > BUFFER_SIZE)
		{
			int rc = EVP_DecryptUpdate(
				ctx.get(), 
				reinterpret_cast<unsigned char*>(buffer), 
				&out_len, 
				reinterpret_cast<const unsigned char*>(data), 
				BUFFER_SIZE
			);
			if (rc != 1)
				return false;
			assert(out_len <= BUFFER_SIZE + BLOCK_SIZE);
			n -= BUFFER_SIZE;
			this->content.append(buffer, out_len);
		}

		int rc = EVP_DecryptUpdate(
			ctx.get(), 
			reinterpret_cast<unsigned char*>(buffer), 
			&out_len, 
			reinterpret_cast<const unsigned char*>(data), 
			static_cast<int>(n)
		);
		if (rc != 1)
			return false;
		
		assert(out_len <= BUFFER_SIZE + BLOCK_SIZE);
		n -= BUFFER_SIZE;
		this->content.append(buffer, out_len);
		return true;
	}

	void DecryptFinal()
	{
		constexpr size_t BLOCK_SIZE = 16;
		char buffer[BLOCK_SIZE];	// NOLINT(*-avoid-c-arrays)
		int out_len = BLOCK_SIZE;
		int rc = EVP_DecryptFinal_ex(
			ctx.get(),
			reinterpret_cast<unsigned char*>(buffer),
			&out_len
		);
		if (rc != 1)
			throw AESError("EVP_DecryptFinal_ex failed");
		assert(out_len <= BLOCK_SIZE);
		this->content.append(buffer, out_len);
	}
};

}



json SekaiNetworkClient::GetAssetList()
{
	string host;
	string AssetVersion;
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		host = "production-" + this->_AssetBundleHostHash + "-assetbundle-info.sekai.colorfulpalette.org";
		AssetVersion = this->_AssetVersion;
	}

	httplib::Client cli("https://" + host);
	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
	{
		cli.set_proxy(this->_ProxyHost, this->_ProxyPort);
	}

	cli.set_connection_timeout(60);		// NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(60);		// NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(300);		// NOLINT(*-avoid-magic-numbers)

	cli.set_default_headers({
		// {"Host", host},
		{"Accept", "*/*"},
		{"Accept-Encoding", "deflate, gzip"},
		{"User-Agent", "ProductName/94 CFNetwork/1335.0.3 Darwin/21.6.0"},
		{"X-Unity-Version", X_UNITY_VERSION}
	});

	/* Get Timestamp */

	using namespace std::chrono;
	auto tp = system_clock::now();
	std::time_t t = system_clock::to_time_t(tp + 1h);	// To JST

	constexpr size_t BUFFER_SIZE = 32;

	char timestamp[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)
	std::strftime(timestamp, BUFFER_SIZE, "%Y%m%d%H%M%S", std::localtime(&t));

	/* Read response */

	AESContentReceiver receiver(this->_AESKey, this->_AESIV);
	
	auto result = cli.Get(
		"/api/version/" + AssetVersion + "/os/ios?t=" + string(timestamp),
		{{"Cookie", this->_GetCookie()}},
		[&receiver](auto* data, auto n) { return receiver.Receiver(data, n); }
	);
	this->_VerifyAndThrow(result);
	receiver.DecryptFinal();
	return json::from_msgpack(std::move(receiver.content));
}



string SekaiNetworkClient::GetMasterSuiteInfo()
{
	auto [id, token] = this->_GetToken();
	auto header = this->_GetHeader();
	header.emplace("X-Session-Token", std::move(token));

	AESContentReceiver receiver(this->_AESKey, this->_AESIV);

	auto result = this->_ApiCli.Get(
		"/api/suite/master", std::move(header),
		[&receiver](auto* data, auto n) { return receiver.Receiver(data, n); }
	);
	this->_VerifyAndThrow(result);
	receiver.DecryptFinal();

	if (result && result->has_header("X-Session-Token"))
		this->_RefreshToken(id, result->get_header_value("X-Session-Token"));
	else
		this->_RefreshToken(id);
	return receiver.content;
}


}