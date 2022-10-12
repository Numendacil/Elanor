#ifndef _UTILS_NETWORK_UTILS_HPP_
#define _UTILS_NETWORK_UTILS_HPP_

#include <optional>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <libmirai/mirai.hpp>

namespace Utils
{

class NetworkException : public std::runtime_error
{
public:
	const std::string _reason;
	const std::string _body;
	const int _code;
	NetworkException(const httplib::Result& result)
		: std::runtime_error("Network error: " + 
		((result)? "Reason: " + result->reason + ", Body: " + result->body + " <" + std::to_string(result->status) + ">"
		: "Reason: " + httplib::to_string(result.error()) + " <-1>")),
		_reason(result ? result->reason : httplib::to_string(result.error())),
		_body(result ? result->body : ""),
		_code(result ? result->status : -1)
	{
	}
};

class ParseError : public std::runtime_error
{
public:
	const std::string _message;
	const std::string _error;
	ParseError(const std::string& error, const std::string& message)
		: std::runtime_error("Unable to parse \"" + message + "\": " + error), _message(message), _error(error)
	{
	}
};

inline bool VerifyResponse(const httplib::Result& result)
{
	if (!result || result.error() != httplib::Error::Success)
		return false;
	if (result->status < 200 || result->status > 299)	// NOLINT(*-avoid-magic-numbers)
		return false;
	return true;
}


inline nlohmann::json GetJsonResponse(const httplib::Result& result)
{
	using json = nlohmann::json;

	if (!VerifyResponse(result))
		throw NetworkException(result);
	
	try
	{
		return json::parse(result->body);
	}
	catch (const json::parse_error& e)
	{
		throw ParseError(e.what(), result->body);
	}
}

}


#endif