#ifndef _LOOP_TRIGGER_BASE_HPP_
#define _LOOP_TRIGGER_BASE_HPP_

#include <chrono>
#include <mutex>
#include <Utils/Timer.hpp>
#include <Trigger/TriggerBase.hpp>

namespace Trigger
{

class LoopTriggerBase : public TriggerBase
{
protected:
	mutable std::mutex mtx;

	virtual void Action() = 0;
	size_t timer_id;

	virtual std::chrono::milliseconds LoopInterval() { return std::chrono::hours(1); }
	virtual bool RandStart() { return false; }

public:
	virtual void TriggerOn() override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->is_running)
			return;
		this->is_running = true;
		this->timer_id = Utils::Timer::GetInstance().LaunchLoop(
			[this]
			{
				this->Action();
			},
			this->LoopInterval(),
			this->RandStart());
	}

	virtual void TriggerOff() override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running)
			return;
		this->is_running = false;
		Utils::Timer::GetInstance().Stop(this->timer_id);
	}

	~LoopTriggerBase()
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->is_running)
		{
			this->is_running = false;
			Utils::Timer::GetInstance().Stop(this->timer_id);
		}
	}
};

}

#endif