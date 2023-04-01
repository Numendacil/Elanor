#include <algorithm>
#include <memory>
#include <string>
#include "Activity.hpp"

namespace State
{

std::unique_ptr<Activity::Holder> Activity::CheckAndStart(const std::string &name)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	if (this->hasActivity)
		return nullptr;
	this->hasActivity = true;
	this->ActivityName = name;
	return std::make_unique<Holder>(this, name);
}

bool Activity::HasActivity() const
{
	std::lock_guard<std::mutex> lk(this->mtx);
	return this->hasActivity;
}

std::string Activity::GetActivityName() const
{
	std::lock_guard<std::mutex> lk(this->mtx);
	return this->ActivityName;
}

bool Activity::WaitForAnswer(Activity::AnswerInfo& answer)
{
	std::unique_lock<std::mutex> lk(this->mtx);
	cv.wait(lk, [this]{
		return !this->answers.empty() || !this->hasActivity;
	});
	if (!this->hasActivity)
		return false;
	answer = this->answers.front();
	this->answers.pop_front();
	return true;
}

bool Activity::WaitForAnswerUntil(std::chrono::time_point<std::chrono::system_clock> due, Activity::AnswerInfo& answer)
{
	std::unique_lock<std::mutex> lk(this->mtx);
	if (!cv.wait_until(lk, due, [this]{
		return !this->answers.empty() || !this->hasActivity;
	}))
		return false;
	if (!this->hasActivity)
		return false;
	answer = this->answers.front();
	this->answers.pop_front();
	return true;
}

void Activity::AddAnswer(const Activity::AnswerInfo& answer)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	if (!this->hasActivity)
		return;
	this->answers.push_back(answer);
	cv.notify_all();
}

}