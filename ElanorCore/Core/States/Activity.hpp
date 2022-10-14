#ifndef _ELANOR_CORE_ACTIVITY_HPP_
#define _ELANOR_CORE_ACTIVITY_HPP_

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

#include <libmirai/Types/BasicTypes.hpp>

#include "StateBase.hpp"

namespace State
{

class Activity : public StateBase
{
protected:
	mutable std::mutex _mtx;
	mutable std::condition_variable _cv;

	bool _hasActivity = false;
	std::string _ActivityName;
	std::deque<nlohmann::json> _answers;

public:
	static constexpr std::string_view _NAME_ = "Activity";

	class Token
	{
	private:
		Activity* _obj;
		std::string _name;

	public:
		Token(Activity* activity, std::string str) : _obj(activity), _name(std::move(str)) {}
		Token(const Token&) = delete;
		Token& operator=(const Token&) = delete;
		Token(Token&&) = delete;
		Token& operator=(Token&&) = delete;

		~Token()
		{
			std::lock_guard<std::mutex> lk(this->_obj->_mtx);
			if (this->_name != this->_obj->_ActivityName) return;
			this->_obj->_hasActivity = false;
			this->_obj->_answers.clear();
			this->_obj->_cv.notify_all();
		}
	};

	std::unique_ptr<Token> CheckAndStart(const std::string& name);
	bool HasActivity() const;

	std::string GetActivityName() const;

	bool WaitForAnswer(nlohmann::json& answer);
	bool WaitForAnswerUntil(std::chrono::time_point<std::chrono::system_clock> due, nlohmann::json& answer);

	void AddAnswer(nlohmann::json answer);
};

} // namespace State

#endif