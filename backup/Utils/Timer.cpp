#include <Utils/Utils.hpp>
#include <ThirdParty/croncpp.h>
#include <ThirdParty/log.h>

#include "Timer.hpp"

namespace Utils
{

std::size_t Timer::GetWorker(void)
{
	this->id_count++;
	for (size_t i = 0; i < this->worker.size(); ++i)
	{
		if (this->worker[i].second.finished)
		{
			if (this->worker[i].first.joinable())
				this->worker[i].first.join();
			this->worker[i].second = {this->id_count, false, false};
			return i;
		}
	}
	this->worker.push_back(make_pair<std::thread, WorkerState>(std::thread(), {this->id_count, false, false}));
	return this->worker.size() - 1;
}



std::size_t Timer::LaunchOnce(std::function<void()> func, std::chrono::milliseconds delay)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = std::thread([this, func, delay, idx]
	{
		{
			std::unique_lock<std::mutex> lk(this->mtx);
			if (cv.wait_for(lk, delay, [this, idx]{ return this->worker[idx].second.stop;}))
				return;
		}

		try
		{
			func();
		}
		catch (const std::exception& e)
		{
			logging::WARN("Exception occured <Timer::LaunchOnce>: " + std::string(e.what()));
		}

		{
			std::lock_guard<std::mutex> lk(this->mtx);
			this->worker[idx].second.finished = true;
		}
	});
	return this->worker[idx].second.id;
}



std::size_t Timer::LaunchLoop(std::function<void()> func, std::chrono::milliseconds interval, bool RandStart)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = std::thread([this, func, interval, RandStart, idx]
	{
		if (RandStart)
		{
			std::uniform_real_distribution<float> dist(0, 1);
			auto pre = interval * dist(Utils::rng_engine);
			{
				std::unique_lock<std::mutex> lk(this->mtx);
				if (cv.wait_for(lk, pre, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}

		while (true)
		{
			try
			{
				func();
			}
			catch (const std::exception &e)
			{
				logging::WARN("Exception occured <Timer::LaunchLoop>: " + std::string(e.what()));
			}

			{
				std::unique_lock<std::mutex> lk(this->mtx);
				if (cv.wait_for(lk, interval, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
	});
	return this->worker[idx].second.id;
}



std::size_t Timer::LaunchAt(std::function<void()> func, const std::string& cron_str, int num)
{
	auto crontab = cron::make_cron(cron_str);
	std::lock_guard<std::mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = std::thread([this, func, crontab, num, idx]
	{
		int count = 0;
		while (true)
		{
			auto t = cron::cron_next(crontab, std::chrono::system_clock::now());
			{
				std::unique_lock<std::mutex> lk(this->mtx);
				if (cv.wait_until(lk, t, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}

			try
			{
				func();
			}
			catch (const std::exception &e)
			{
				logging::WARN("Exception occured <Timer::LaunchAt>: " + std::string(e.what()));
			}

			if (num > 0)
			{
				count++;
				if (count >= num)
				{
					std::unique_lock<std::mutex> lk(this->mtx);
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
	});
	return this->worker[idx].second.id;
}

}