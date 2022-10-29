#ifndef _IMAGE_SEARCH_SAUCENAO_CLIENT_HPP_
#define _IMAGE_SEARCH_SAUCENAO_CLIENT_HPP_

#include <string>
#include <string_view>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "Models.hpp"

namespace SauceNAO
{

class SauceClient
{
	static constexpr std::string_view api_host = "https://saucenao.com";

	static constexpr size_t DEFAULT_RESULTNUM = 5;
	static constexpr float DEFAULT_SIMULARITY = 70.0;

public:
	enum OUTPUT_TYPE { HTML = 0, XML, JSON };
	enum DEDUPE_LEVEL { NONE = 0, BY_ID, BEST_EFFORT };
	enum HIDE_EXPLICIT { SHOW_ALL = 0, HIDE_EXPECTED, HIDE_SUSPECTED, SHOW_EXPECTED_SAFE };

	struct SearchOptions
	{
		float MinSimularity = DEFAULT_SIMULARITY;
		uint64_t mask = SauceNAO::MASK_ALL;
		HIDE_EXPLICIT hide = HIDE_EXPECTED;
		size_t ResultNum = DEFAULT_RESULTNUM;
		bool TestMode = false;
		DEDUPE_LEVEL dedupe = BEST_EFFORT;
	};

protected:
	const std::string _APIKey;
	const OUTPUT_TYPE _type = JSON;

	const size_t _ResultNum;
	const bool _TestMode;
	const float _MinSimularity;
	const uint64_t _mask;
	const DEDUPE_LEVEL _dedupe;
	const HIDE_EXPLICIT _hide;

	httplib::Client _cli;

public:
	explicit SauceClient(
		std::string APIKey,
		const SearchOptions& opts,
		const std::string& ProxyHost = {},
		int ProxyPort = -1
	);

	SauceNAOResult SearchFile(std::string content);
	SauceNAOResult SearchUrl(const std::string& url);

};

}


#endif