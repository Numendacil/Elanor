#ifndef _ELANOR_CORE_UTILS_COMMON_HPP_
#define _ELANOR_CORE_UTILS_COMMON_HPP_

#include <mutex>
#include <random>
#include <optional>
#include <nlohmann/json.hpp>

namespace Utils
{

std::mt19937& GetRngEngine();

class BotConfig
{
private:
	nlohmann::json _config;
	mutable std::mutex _mtx;

public:
	BotConfig() = default;
	BotConfig(nlohmann::json config) : _config(std::move(config)) {}
	BotConfig& operator=(const BotConfig& rhs)
	{
		if (this == &rhs) return *this;

		std::scoped_lock lk(this->_mtx, rhs._mtx);
		this->_config = rhs._config;
		return *this;
	}
	BotConfig(const BotConfig& rhs) { *this = rhs; }
	BotConfig& operator=(BotConfig&& rhs)
	{
		if (this == &rhs) return *this;

		std::scoped_lock lk(this->_mtx, rhs._mtx);
		this->_config = std::move(rhs._config);
		return *this;
	}
	BotConfig(BotConfig&& rhs) { *this = rhs; }
	bool FromFile(const std::string& filepath);

	template<typename T>
	T Get(const std::string& key, const T& value) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto jp = nlohmann::json::json_pointer(key);
		if (this->_config.contains(jp))
			return this->_config.at(jp).get<T>();
		else
			return value;
	}
	template<typename T>
	std::optional<T> Get(const std::string& key) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto jp = nlohmann::json::json_pointer(key);
		if (this->_config.contains(jp))
			return this->_config.at(jp).get<T>();
		else
			return std::nullopt;
	}
};

}

#endif