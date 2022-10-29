#include "SauceClient.hpp"

#include <string>
#include <PluginUtils/NetworkUtils.hpp>
#include "SauceNAO/Models.hpp"
#include "httplib.h"

using std::string;
using json = nlohmann::json;

namespace SauceNAO
{

SauceClient::SauceClient(
	std::string APIKey,
	const SearchOptions& opts,
	const std::string& ProxyHost,
	int ProxyPort
) : _APIKey(std::move(APIKey)), _ResultNum(opts.ResultNum),
	_TestMode(opts.TestMode), _MinSimularity(opts.MinSimularity),
	_mask(opts.mask), _dedupe(opts.dedupe), _hide(opts.hide),
	_cli(api_host.data())
{
	if (!ProxyHost.empty() && ProxyPort > 0)
		this->_cli.set_proxy(ProxyHost, ProxyPort);

	this->_cli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	this->_cli.set_write_timeout(120); // NOLINT(*-avoid-magic-numbers)
	this->_cli.set_read_timeout(120); // NOLINT(*-avoid-magic-numbers)
}

SauceNAOResult SauceClient::SearchFile(std::string content)
{
	httplib::MultipartFormDataItems payload{
		{"output_type", std::to_string(this->_type), "", ""},
		{"api_key", this->_APIKey, "", ""},
		{"testmode", this->_TestMode ? "1" : "0", "", ""},
		{"numres", std::to_string(this->_ResultNum), "", ""},
		{"hide",  std::to_string(this->_hide), "", ""},
		{"minsim", std::to_string(this->_MinSimularity), "", ""},
		{"dedupe", std::to_string(this->_dedupe), "", ""},
		{"file", std::move(content), "", "application/octet-stream"}
	};
	if (this->_mask == MASK_ALL)
		payload.emplace_back("db", "999", "", "");
	else
		payload.emplace_back("dbmask", std::to_string(this->_mask), "", "");

	auto result = this->_cli.Post(
		"/search.php",
		payload
	);
	return Utils::GetJsonResponse(result);
}

SauceNAOResult SauceClient::SearchUrl(const std::string& url)
{

	httplib::Params params{
		{"output_type", std::to_string(this->_type)},
		{"api_key", this->_APIKey},
		{"testmode", this->_TestMode ? "1" : "0"},
		{"numres", std::to_string(this->_ResultNum)},
		{"hide",  std::to_string(this->_hide)},
		{"minsim", std::to_string(this->_MinSimularity)},
		{"dedupe", std::to_string(this->_dedupe)},
		{"url", url}
	};
	if (this->_mask == MASK_ALL)
		params.emplace("db", "999");
	else
		params.emplace("dbmask", std::to_string(this->_mask));
	
	auto result = this->_cli.Post(
		"/search.php",
		params
	);
	return Utils::GetJsonResponse(result);
}

}