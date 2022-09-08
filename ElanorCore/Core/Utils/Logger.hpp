#ifndef _ELANOR_CORE_LOGGER_HPP_
#define _ELANOR_CORE_LOGGER_HPP_

#include <memory>

#include <libmirai/Utils/Logger.hpp>

namespace Utils
{

class Logger : public Mirai::ILogger
{
public:
	void log(const std::string& msg, Mirai::LoggingLevels level) override;
};

std::shared_ptr<Logger> GetLoggerPtr();
Logger& GetLogger();

} // namespace Utils

#endif