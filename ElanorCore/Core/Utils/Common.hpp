#ifndef _ELANOR_CORE_UTILS_COMMON_HPP_
#define _ELANOR_CORE_UTILS_COMMON_HPP_

#include <random>
#include <optional>
#include <nlohmann/json.hpp>

namespace Utils
{

std::mt19937& GetRngEngine();

class BotConfig
{
private:
	nlohmann::json config;

public:
	BotConfig() = default;
	BotConfig(const nlohmann::json& config) { this->config = config;}
	bool FromFile(const std::string& filepath);

	template<typename T>
	T Get(const nlohmann::json::json_pointer& key, const T& value) const
	{
		if (this->config.contains(key))
			return this->config.at(key).get<T>();
		else
			return value;
	}
	template<typename T>
	std::optional<T> Get(const nlohmann::json::json_pointer& key) const
	{
		if (this->config.contains(key))
			return this->config.at(key).get<T>();
		else
			return std::nullopt;
	}
};

}

#endif