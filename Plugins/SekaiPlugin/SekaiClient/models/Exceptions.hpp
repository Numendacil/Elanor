#ifndef _SEKAI_EXCEPTIONS_HPP_
#define _SEKAI_EXCEPTIONS_HPP_

#include <stdexcept>


namespace Sekai
{

class AESError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class FileError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class UpdateError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class CookieError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class NetworkException : public std::runtime_error
{
public:
	const std::string _reason;
	const std::string _body;
	const int _code;
	NetworkException(std::string reason, std::string body, int code)
		: std::runtime_error("Network error: \nReason: " + reason + ", Body: " + body + " <" + std::to_string(code)
	                         + ">")
		, _reason(std::move(reason))
		, _body(std::move(body))
		, _code(code)
	{
	}
};

class UpgradeRequired : public NetworkException
{
public:
	// NOLINTNEXTLINE(*-avoid-magic-numbers)
	UpgradeRequired() : NetworkException("Upgrade required", "", 426) {}
};

} // namespace Sekai

#endif