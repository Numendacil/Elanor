#ifndef LOG_HPP
#define LOG_HPP
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <unordered_map>
#include <memory>

#include "date.h"

namespace logging
{

	//TODO: use macros (again) so __FILE__ __LINE__ could be automatically added to certain error levels?

	//the log levels we support
	enum class log_level : uint8_t
	{
		TRACE = 0,
		DEBUG = 1,
		INFO = 2,
		WARN = 3,
		ERROR = 4
	};
	struct enum_hasher
	{
		template <typename T>
		std::size_t operator()(T t) const { return static_cast<std::size_t>(t); }
	};
	const std::unordered_map<log_level, std::string, enum_hasher> uncolored{
		{log_level::ERROR, " [ERROR] "}, 
		{log_level::WARN, " [WARN] "}, 
		{log_level::INFO, " [INFO] "}, 
		{log_level::DEBUG, " [DEBUG] "}, 
		{log_level::TRACE, " [TRACE] "}
		};
	const std::unordered_map<log_level, std::string, enum_hasher> colored{
		{log_level::ERROR, " \x1b[31m[ERROR]\x1b[0m "}, 
		{log_level::WARN, " \x1b[33m[WARN]\x1b[0m "}, 
		{log_level::INFO, " \x1b[32m[INFO]\x1b[0m "}, 
		{log_level::DEBUG, " \x1b[34m[DEBUG]\x1b[0m "}, 
		{log_level::TRACE, " \x1b[37m[TRACE]\x1b[0m "}
		};

//all, something in between, none or default to info
#if defined(LOGGING_LEVEL_ALL) || defined(LOGGING_LEVEL_TRACE)
	constexpr log_level LOG_LEVEL_CUTOFF = log_level::TRACE;
#elif defined(LOGGING_LEVEL_DEBUG)
	constexpr log_level LOG_LEVEL_CUTOFF = log_level::DEBUG;
#elif defined(LOGGING_LEVEL_WARN)
	constexpr log_level LOG_LEVEL_CUTOFF = log_level::WARN;
#elif defined(LOGGING_LEVEL_ERROR)
	constexpr log_level LOG_LEVEL_CUTOFF = log_level::ERROR;
#elif defined(LOGGING_LEVEL_NONE)
	constexpr log_level LOG_LEVEL_CUTOFF = log_level::ERROR + 1;
#else
	constexpr log_level LOG_LEVEL_CUTOFF = log_level::INFO;
#endif

	//returns formated to: 'year/mo/dy hr:mn:sc.xxxxxx'
	inline std::string timestamp()
	{
		using namespace date;
		using namespace std::chrono;
		auto tp = system_clock::now();
		tp += 8h;
		std::stringstream ss;
		ss << "\x1b[95;1m" << floor<seconds>(tp) << "\x1b[0m";
		return ss.str();
	}

	//logger base class, not pure virtual so you can use as a null logger if you want
	using logging_config_t = std::unordered_map<std::string, std::string>;
	class logger
	{
	public:
		logger() = delete;
		logger(const logging_config_t &){};
		virtual ~logger(){};
		virtual void log(const std::string &, const log_level){};
		virtual void log(const std::string &){};

	protected:
		std::mutex lock;
	};

	//logger that writes to standard out
	class std_out_logger : public logger
	{
	public:
		std_out_logger() = delete;
		std_out_logger(const logging_config_t &config) : logger(config), levels(config.find("color") != config.end() ? colored : uncolored) {}
		virtual void log(const std::string &message, const log_level level)
		{
			if (level < LOG_LEVEL_CUTOFF)
				return;
			std::string output;
			output.reserve(message.length() + 64);
			output.append(timestamp());
			output.append(levels.find(level)->second);
			output.append(message);
			output.push_back('\n');
			log(output);
		}
		virtual void log(const std::string &message)
		{
			//cout is thread safe, to avoid multiple threads interleaving on one line
			//though, we make sure to only call the << operator once on std::cout
			//otherwise the << operators from different threads could interleave
			//obviously we dont care if flushes interleave
			//std::lock_guard<std::mutex> lk{lock};
			std::cout << message;
			std::cout.flush();
		}

	protected:
		const std::unordered_map<log_level, std::string, enum_hasher> levels;
	};

	//TODO: add log rolling
	//logger that writes to file
	class file_logger : public logger
	{
	public:
		file_logger() = delete;
		file_logger(const logging_config_t &config) : logger(config)
		{
			//grab the file name
			auto name = config.find("file_name");
			if (name == config.end())
				throw std::runtime_error("No output file provided to file logger");
			file_name = name->second;

			//if we specify an interval
			reopen_interval = std::chrono::seconds(300);
			auto interval = config.find("reopen_interval");
			if (interval != config.end())
			{
				try
				{
					reopen_interval = std::chrono::seconds(std::stoul(interval->second));
				}
				catch (...)
				{
					throw std::runtime_error(interval->second + " is not a valid reopen interval");
				}
			}

			//crack the file open
			reopen();
		}
		virtual void log(const std::string &message, const log_level level)
		{
			if (level < LOG_LEVEL_CUTOFF)
				return;
			std::string output;
			output.reserve(message.length() + 64);
			output.append(timestamp());
			output.append(uncolored.find(level)->second);
			output.append(message);
			output.push_back('\n');
			log(output);
		}
		virtual void log(const std::string &message)
		{
			lock.lock();
			file << message;
			file.flush();
			lock.unlock();
			reopen();
		}

	protected:
		void reopen()
		{
			//TODO: use CLOCK_MONOTONIC_COARSE
			//check if it should be closed and reopened
			auto now = std::chrono::system_clock::now();
			lock.lock();
			if (now - last_reopen > reopen_interval)
			{
				last_reopen = now;
				try
				{
					file.close();
				}
				catch (...)
				{
				}
				try
				{
					file.open(file_name, std::ofstream::out | std::ofstream::app);
					last_reopen = std::chrono::system_clock::now();
				}
				catch (std::exception &e)
				{
					try
					{
						file.close();
					}
					catch (...)
					{
					}
					throw e;
				}
			}
			lock.unlock();
		}
		std::string file_name;
		std::ofstream file;
		std::chrono::seconds reopen_interval;
		std::chrono::system_clock::time_point last_reopen;
	};

	//a factory that can create loggers (that derive from 'logger') via function pointers
	//this way you could make your own logger that sends log messages to who knows where
	using logger_creator = logger *(*)(const logging_config_t &);
	class logger_factory
	{
	public:
		logger_factory()
		{
			creators.emplace("", [](const logging_config_t &config) -> logger * { return new logger(config); });
			creators.emplace("std_out", [](const logging_config_t &config) -> logger * { return new std_out_logger(config); });
			creators.emplace("file", [](const logging_config_t &config) -> logger * { return new file_logger(config); });
		}
		logger *produce(const logging_config_t &config) const
		{
			//grab the type
			auto type = config.find("type");
			if (type == config.end())
				throw std::runtime_error("Logging factory configuration requires a type of logger");
			//grab the logger
			auto found = creators.find(type->second);
			if (found != creators.end())
				return found->second(config);
			//couldn't get a logger
			throw std::runtime_error("Couldn't produce logger for type: " + type->second);
		}

	protected:
		std::unordered_map<std::string, logger_creator> creators;
	};

	//statically get a factory
	inline logger_factory &get_factory()
	{
		static logger_factory factory_singleton{};
		return factory_singleton;
	}

	//get at the singleton
	inline logger &get_logger(const logging_config_t &config = {{"type", "std_out"}, {"color", ""}})
	{
		static std::unique_ptr<logger> singleton(get_factory().produce(config));
		return *singleton;
	}

	//configure the singleton (once only)
	inline void configure(const logging_config_t &config)
	{
		get_logger(config);
	}

	//statically log manually without the macros below
	inline void log(const std::string &message, const log_level level)
	{
		get_logger().log(message, level);
	}

	//statically log manually without a level or maybe with a custom one
	inline void log(const std::string &message)
	{
		get_logger().log(message);
	}

	//these standout when reading code
	inline void TRACE(const std::string &message)
	{
		get_logger().log(message, log_level::TRACE);
	}
	inline void DEBUG(const std::string &message)
	{
		get_logger().log(message, log_level::DEBUG);
	}
	inline void INFO(const std::string &message)
	{
		get_logger().log(message, log_level::INFO);
	}
	inline void WARN(const std::string &message)
	{
		get_logger().log(message, log_level::WARN);
	}
	inline void ERROR(const std::string &message)
	{
		get_logger().log(message, log_level::ERROR);
	}
}

#endif