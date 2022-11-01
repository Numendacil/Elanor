#ifndef _ELANOR_CORE_UTILS_COMMON_HPP_
#define _ELANOR_CORE_UTILS_COMMON_HPP_

#include <mutex>
#include <optional>
#include <random>

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
	~BotConfig() = default;

	bool FromFile(const std::string& filepath);

	template<typename ValueType, typename KeyType>
	auto Get(KeyType&& key, ValueType&& value) const
	{
		using ReturnType = decltype(std::declval<typename nlohmann::json>().value(std::forward<KeyType>(key),
		                                                                          std::forward<ValueType>(value)));
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto jp = nlohmann::json::json_pointer(key);
		if (this->_config.contains(jp)) 
			return this->_config.at(jp).template get<ReturnType>();
		else
			return (ReturnType)std::forward<ValueType>(value);
	}

	template<typename ValueType, typename KeyType>
	std::optional<ValueType> Get(KeyType&& key) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto jp = nlohmann::json::json_pointer(key);
		if (this->_config.contains(jp)) return this->_config.at(jp).get<ValueType>();
		else
			return std::nullopt;
	}

	template <typename KeyType>
	bool exist(KeyType&& key) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto jp = nlohmann::json::json_pointer(key);
		return this->_config.contains(jp);
	}

	template <typename KeyType>
	bool IsNull(KeyType&& key) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		auto jp = nlohmann::json::json_pointer(key);
		if (this->_config.contains(jp)) 
			return this->_config.at(jp).is_null();
		else
			return false;

	}
};

} // namespace Utils

#endif