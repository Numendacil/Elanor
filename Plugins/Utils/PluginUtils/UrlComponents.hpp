#ifndef _UTILS_URL_COMPONENTS_HPP_
#define _UTILS_URL_COMPONENTS_HPP_

#include <filesystem>
#include <string>
#include <regex>
#include <charconv>

#include "StringUtils.hpp"

namespace Utils
{

struct UrlComponent
{
	std::string scheme;
	std::string authority;
	std::string path;
	std::string query;
	std::string fragment;

	static UrlComponent ParseUrl(const std::string& url)
	{
		static const std::regex reg_url(R"(^(?:([^:/?#]+):)?(?://([^/?#]*))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

		std::smatch result;
		if (!std::regex_match(url, result, reg_url) || result.size() < 6)	// NOLINT(*-avoid-magic-numbers)
		{
			return UrlComponent{};
		}

		return UrlComponent {
			toLower(result[1].str()), 
			toLower(result[2].str()), 
			result[3].str(),
			result[4].str(), 
			result[5].str()		// NOLINT(*-avoid-magic-numbers)
		};
	}

	std::pair<std::string, int> GetHostPost() const
	{
		static const std::unordered_map<std::string, int> default_ports = {
			{"acap", 674}, 
			{"afp", 548}, 
			{"dict", 2628}, 
			{"dns", 53},
			{"ftp", 21}, 
			{"git", 9418}, 
			{"gopher", 70}, 
			{"http", 80}, 
			{"https", 443}, 
			{"imap", 143}, 
			{"ipp", 631}, 
			{"ipps", 631}, 
			{"irc", 194}, 
			{"ircs", 6697}, 
			{"ldap", 389}, 
			{"ldaps", 636}, 
			{"mms", 1755}, 
			{"msrp", 2855},
			{"mtqp", 1038}, 
			{"nfs", 111}, 
			{"nntp", 119}, 
			{"nntps", 563}, 
			{"pop", 110}, 
			{"prospero", 1525}, 
			{"redis", 6379}, 
			{"rsync", 873}, 
			{"rtsp", 554}, 
			{"rtsps", 322}, 
			{"rtspu", 5005}, 
			{"sftp", 22}, 
			{"smb", 445}, 
			{"snmp", 161}, 
			{"ssh", 22},
			{"svn", 3690}, 
			{"telnet", 23}, 
			{"ventrilo", 3784}, 
			{"vnc", 5900}, 
			{"wais", 210}, 
			{"ws", 80}, 
			{"wss", 443}
		};

		std::string_view str = this->authority;
		auto n = str.find_last_of(":");
		if (n == std::string_view::npos)
		{
			auto n = default_ports.find(this->scheme);
			if (n == default_ports.end())
				return std::make_pair(this->authority, 0);
			else
				return std::make_pair(this->authority, n->second);
		}

		int port{};
		std::string_view port_str = str.substr(n + 1);
		auto result = std::from_chars(port_str.data(), port_str.data() + port_str.size(), port);
		if (result.ec != std::errc())
			return std::make_pair(this->authority.substr(0, n), -1);

		return std::make_pair(this->authority.substr(0, n), port);
	}

	std::string GetFilename() const
	{
		std::filesystem::path fpath = this->path;
		return fpath.filename().string();
	}

	std::string GetOrigin() const
	{
		return (scheme.empty() ?  "" : scheme + ":") + (authority.empty() ? "" : "//" + authority);
	}

	std::string GetUrl() const
	{
		return (scheme.empty() ?  "" : scheme + ":") 
			+ (authority.empty() ? "" : "//" + authority)
			+ (path)
			+ (query.empty() ? "" : "?" + query)
			+ (fragment.empty() ? "" : "#" + fragment);
	}

	std::string GetRelativeRef() const
	{
		return (path)
			+ (query.empty() ? "" : "?" + query)
			+ (fragment.empty() ? "" : "#" + fragment);
	}

};


}

#endif