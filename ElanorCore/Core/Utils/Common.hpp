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
	T Get(const std::string& key, const T& value) const
	{
		auto jp = nlohmann::json::json_pointer(key);
		if (this->config.contains(jp))
			return this->config.at(jp).get<T>();
		else
			return value;
	}
	template<typename T>
	std::optional<T> Get(const std::string& key) const
	{
		auto jp = nlohmann::json::json_pointer(key);
		if (this->config.contains(jp))
			return this->config.at(jp).get<T>();
		else
			return std::nullopt;
	}
};

}

#endif