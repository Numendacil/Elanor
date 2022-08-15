#ifndef _CRON_TRIGGER_BASE_HPP_
#define _CRON_TRIGGER_BASE_HPP_

#include <mutex>
#include <Utils/Timer.hpp>
#include <Trigger/TriggerBase.hpp>

namespace Trigger
{

class CronTriggerBase : public TriggerBase
{
protected:
	mutable std::mutex mtx;

	virtual void Action() = 0;
	size_t timer_id;

	virtual std::string GetCron() { return "0 0 8 * * *"; }

public:
	virtual void TriggerOn() override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->is_running)
			return;
		this->is_running = true;
		this->timer_id = Utils::Timer::GetInstance().LaunchAt(
			[this]
			{
				this->Action();
			},
			this->GetCron());
	}

	virtual void TriggerOff(void) override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running)
			return;
		this->is_running = false;
		Utils::Timer::GetInstance().Stop(this->timer_id);
	}

	~CronTriggerBase()
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