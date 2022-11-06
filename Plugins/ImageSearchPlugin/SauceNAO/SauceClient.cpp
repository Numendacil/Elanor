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
	SearchOptions opts,
	std::string ProxyHost,
	int ProxyPort
) 
: _APIKey(std::move(APIKey)), _opts(std::move(opts)),
  _ProxyHost(std::move(ProxyHost)), _ProxyPort(ProxyPort)
{
}

httplib::Client SauceClient::_GetClient() const
{
	httplib::Client cli(api_host.data());
	if (!this->_ProxyHost.empty() && this->_ProxyPort > 0)
		cli.set_proxy(this->_ProxyHost, this->_ProxyPort);

	cli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(120); // NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(120); // NOLINT(*-avoid-magic-numbers)
	cli.set_default_headers({
		{"Accept", "*/*"},
		{"Accept-Encoding", "gzip, deflate"},
		{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
	});

	return cli;
}

SauceNAOResult SauceClient::SearchFile(std::string content, std::string filename)
{
	Utils::Params params{
		{"output_type", std::to_string(this->_type)},
		{"api_key", this->_APIKey},
		{"testmode", this->_opts.TestMode ? "1" : "0"},
		{"numres", std::to_string(this->_opts.ResultNum)},
		{"hide",  std::to_string(this->_opts.hide)},
		{"minsim", std::to_string(this->_opts.MinSimilarity)},
		{"dedupe", std::to_string(this->_opts.dedupe)}
	};
	if (this->_opts.mask == MASK_ALL)
		params.emplace("db", "999");
	else
		params.emplace("dbmask", std::to_string(this->_opts.mask));

	httplib::MultipartFormDataItems payload;
	payload.emplace_back(httplib::MultipartFormData{"file", std::move(content), std::move(filename), "application/octet-stream"});

	auto result = this->_GetClient().Post(
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
		{"testmode", this->_opts.TestMode ? "1" : "0"},
		{"numres", std::to_string(this->_opts.ResultNum)},
		{"hide",  std::to_string(this->_opts.hide)},
		{"minsim", std::to_string(this->_opts.MinSimilarity)},
		{"dedupe", std::to_string(this->_opts.dedupe)},
	};
	params.emplace("url", std::move(url));
	if (this->_opts.mask == MASK_ALL)
		params.emplace("db", "999");
	else
		params.emplace("dbmask", std::to_string(this->_opts.mask));
	
	auto result = this->_GetClient().Post(
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