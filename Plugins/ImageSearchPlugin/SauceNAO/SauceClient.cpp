#include "SauceClient.hpp"

#include <string>
#include <utility>
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
	_TestMode(opts.TestMode), _MinSimilarity(opts.MinSimilarity),
	_mask(opts.mask), _dedupe(opts.dedupe), _hide(opts.hide),
	_cli(api_host.data())
{
	if (!ProxyHost.empty() && ProxyPort > 0)
		this->_cli.set_proxy(ProxyHost, ProxyPort);

	this->_cli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	this->_cli.set_write_timeout(120); // NOLINT(*-avoid-magic-numbers)
	this->_cli.set_read_timeout(120); // NOLINT(*-avoid-magic-numbers)
	this->_cli.set_default_headers({
		{"Accept", "*/*"},
		{"Accept-Encoding", "gzip, deflate"},
		{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
	});
}

SauceNAOResult SauceClient::SearchFile(std::string content, std::string filename)
{
	Utils::Params params{
		{"output_type", std::to_string(this->_type)},
		{"api_key", this->_APIKey},
		{"testmode", this->_TestMode ? "1" : "0"},
		{"numres", std::to_string(this->_ResultNum)},
		{"hide",  std::to_string(this->_hide)},
		{"minsim", std::to_string(this->_MinSimilarity)},
		{"dedupe", std::to_string(this->_dedupe)}
	};
	if (this->_mask == MASK_ALL)
		params.emplace("db", "999");
	else
		params.emplace("dbmask", std::to_string(this->_mask));

	httplib::MultipartFormDataItems payload{
		{"file", std::move(content), std::move(filename), "application/octet-stream"}
	};

	auto result = this->_cli.Post(
		"/search.php?" + Utils::Params2Query(params),
		payload
	);

	json resp = Utils::GetJsonResponse(result);
	int status = resp.at("header").at("status").get<int>();
	if (status < 0)
		throw SauceNAOError(resp.at("header").at("message").get<string>(), status);
	return resp.get<SauceNAOResult>();
}

SauceNAOResult SauceClient::SearchFile(const std::filesystem::path& file)
{
	std::string content;
	{	
		std::ifstream ifile(file);

		constexpr size_t BUFFER_SIZE = 4096;
		char buffer[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)
		while(ifile.read(buffer, BUFFER_SIZE))
			content.append(buffer, BUFFER_SIZE);
		content.append(buffer, ifile.gcount());
	}
	return this->SearchFile(std::move(content), file.filename());
}

SauceNAOResult SauceClient::SearchUrl(std::string url)
{
	httplib::Params params{
		{"output_type", std::to_string(this->_type)},
		{"api_key", this->_APIKey},
		{"testmode", this->_TestMode ? "1" : "0"},
		{"numres", std::to_string(this->_ResultNum)},
		{"hide",  std::to_string(this->_hide)},
		{"minsim", std::to_string(this->_MinSimilarity)},
		{"dedupe", std::to_string(this->_dedupe)},
		{"url", std::move(url)}
	};
	if (this->_mask == MASK_ALL)
		params.emplace("db", "999");
	else
		params.emplace("dbmask", std::to_string(this->_mask));
	
	auto result = this->_cli.Post(
		"/search.php",
		params
	);

	json resp = Utils::GetJsonResponse(result);
	int status = resp.at("header").at("status").get<int>();
	if (status < 0)
		throw SauceNAOError(resp.at("header").at("message").get<string>(), status);
	return resp.get<SauceNAOResult>();
}

}