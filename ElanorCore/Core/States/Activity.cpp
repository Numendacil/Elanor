#include "Activity.hpp"

#include <algorithm>
#include <memory>
#include <string>

namespace State
{

std::unique_ptr<Activity::Token> Activity::CheckAndStart(const std::string& name)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	if (this->_hasActivity) return nullptr;
	this->_hasActivity = true;
	this->_ActivityName = name;
	return std::make_unique<Token>(this, name);
}

bool Activity::HasActivity() const
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	return this->_hasActivity;
}

std::string Activity::GetActivityName() const
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	return this->_ActivityName;
}

bool Activity::WaitForAnswer(Activity::AnswerInfo& answer)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	this->_cv.wait(lk, [this] { return !this->_answers.empty() || !this->_hasActivity; });
	if (!this->_hasActivity) return false;
	answer = this->_answers.front();
	this->_answers.pop_front();
	return true;
}

bool Activity::WaitForAnswerUntil(std::chrono::time_point<std::chrono::system_clock> due, Activity::AnswerInfo& answer)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	if (!this->_cv.wait_until(lk, due, [this] { return !this->_answers.empty() || !this->_hasActivity; })) return false;
	if (!this->_hasActivity) return false;
	answer = this->_answers.front();
	this->_answers.pop_front();
	return true;
}

void Activity::AddAnswer(const Activity::AnswerInfo& answer)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	if (!this->_hasActivity) return;
	this->_answers.push_back(answer);
	this->_cv.notify_all();
}

} // namespace State