#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <random>
#include <optional>

#include <nlohmann/json.hpp>
#include <libmirai/mirai.hpp>

namespace httplib
{

class Client;
class Result;

}

namespace Utils
{
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

namespace Utils
{
	extern std::mt19937 rng_engine;
	extern BotConfig Configs;

	void Init(const std::string& config_path);

	std::string exec(const std::vector<std::string>& cmd);

	void ReplaceMark(std::string &);
	void ToLower(std::string &);
	int ToBool(const std::string&);
	int Tokenize(std::vector<std::string>&, std::string, int = -1);

	bool CheckHttpResponse(const httplib::Result& result, const std::string& Caller = "");
	void SetClientOptions(httplib::Client& cli);

	std::string GetDescription(const Mirai::GroupMessageEvent &, bool = true);
	std::string GetDescription(const Mirai::FriendMessageEvent &, bool = true);

	std::string GetText(const Mirai::MessageChain& msg);
}

#endif
