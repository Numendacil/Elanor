#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <string_view>

namespace
{

std::string GetTimestamp()
{
	using namespace std::chrono;
	auto tp = system_clock::now();
	std::time_t t = system_clock::to_time_t(tp);
	char buf[64];
	std::strftime(buf, 64, "\x1b[95m%Y-%m-%d %H:%M:%S\x1b[0m", std::localtime(&t));
	return {buf};
}

constexpr std::string_view GetLevelStr(Mirai::LoggingLevels level)
{
	switch (level)
	{
	case Mirai::LoggingLevels::TRACE:
		return " \x1b[37m[TRACE]\x1b[0m ";
	case Mirai::LoggingLevels::DEBUG:
		return " \x1b[34m[DEBUG]\x1b[0m ";
	case Mirai::LoggingLevels::INFO:
		return " \x1b[32m[INFO]\x1b[0m ";
	case Mirai::LoggingLevels::WARN:
		return " \x1b[33m[WARN]\x1b[0m ";
	case Mirai::LoggingLevels::ERROR:
		return " \x1b[31m[ERROR]\x1b[0m ";
	case Mirai::LoggingLevels::FATAL:
		return " \x1b[91m[FATAL]\x1b[0m ";
	default:
		return "";
	}
}

} // namespace

namespace Utils
{

void Logger::log(const std::string& msg, Mirai::LoggingLevels level)
{
	std::string text;
	text = GetTimestamp() + std::string(GetLevelStr(level)) + msg + '\n';
	std::cout << text;
	std::cout.flush();
}

namespace
{

std::shared_ptr<Logger> logger = std::make_shared<Logger>();

}

std::shared_ptr<Logger> GetLoggerPtr()
{
	return logger;
}

Logger& GetLogger()
{
	return *logger;
}

} // namespace Utils