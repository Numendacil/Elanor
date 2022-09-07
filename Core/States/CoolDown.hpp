#ifndef _ELANOR_CORE_COOLDOWN_HPP_
#define _ELANOR_CORE_COOLDOWN_HPP_

#include <chrono>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "StateBase.hpp"

namespace State
{

class CoolDown : public StateBase
{
protected:
	mutable std::mutex _mtx;

	struct command_state
	{
		bool isUsing = false;
		std::optional<std::chrono::system_clock::time_point> LastUsed = std::nullopt;
	};

	std::unordered_map<std::string, command_state> _cd;

public:
	static constexpr std::string_view _NAME_ = "CoolDown";

	struct Token
	{
	private:
		CoolDown* _obj;
		std::string _id;

	public:
		Token(CoolDown* cd, std::string str) : _obj(cd), _id(std::move(str)) {}
		Token(const Token&) = delete;
		Token& operator=(const Token&) = delete;
		Token(Token&&) = delete;
		Token& operator=(Token&&) = delete;
		~Token()
		{
			std::lock_guard<std::mutex> lk(this->_obj->_mtx);
			this->_obj->_cd[this->_id].isUsing = false;
			this->_obj->_cd[this->_id].LastUsed = std::chrono::system_clock::now();
		}
	};

	std::unique_ptr<Token> GetRemaining(const std::string& _id, const std::chrono::seconds& _cd,
	                                    std::chrono::seconds& remaining);

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& content) override;
};

} // namespace State

#endif