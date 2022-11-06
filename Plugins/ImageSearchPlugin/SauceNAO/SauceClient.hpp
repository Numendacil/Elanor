#ifndef _IMAGE_SEARCH_SAUCENAO_CLIENT_HPP_
#define _IMAGE_SEARCH_SAUCENAO_CLIENT_HPP_

#include <filesystem>
#include <string>
#include <string_view>

#include "Models.hpp"

namespace httplib
{

class Client;

}

namespace SauceNAO
{

class SauceClient
{
	static constexpr std::string_view api_host = "https://saucenao.com";

	static constexpr size_t DEFAULT_RESULTNUM = 5;
	static constexpr double DEFAULT_SIMILARITY = 70.0;

public:
	enum OUTPUT_TYPE { HTML = 0, XML, JSON };
	enum DEDUPE_LEVEL { NONE = 0, BY_ID, BEST_EFFORT };
	enum HIDE_EXPLICIT { SHOW_ALL = 0, HIDE_EXPECTED, HIDE_SUSPECTED, SHOW_EXPECTED_SAFE };

	struct SearchOptions
	{
		double MinSimilarity = DEFAULT_SIMILARITY;
		uint64_t mask = SauceNAO::MASK_ALL;
		HIDE_EXPLICIT hide = HIDE_EXPECTED;
		size_t ResultNum = DEFAULT_RESULTNUM;
		bool TestMode = false;
		DEDUPE_LEVEL dedupe = BEST_EFFORT;
	};

protected:
	const std::string _APIKey;
	const OUTPUT_TYPE _type = JSON;

	const SearchOptions _opts;

	const std::string _ProxyHost;
	const int _ProxyPort;

	httplib::Client _GetClient() const;

public:
	explicit SauceClient(
		std::string APIKey,
		SearchOptions opts,
		std::string ProxyHost = {},
		int ProxyPort = -1
	);

	SauceNAOResult SearchFile(std::string content, std::string filename);
	SauceNAOResult SearchFile(const std::filesystem::path& file);
	SauceNAOResult SearchUrl(std::string url);

};

}


#endif