#include <chrono>
#include <Core/Utils/Logger.hpp>
#include <Core/Utils/Common.hpp>

#include "Timer.hpp"

namespace Utils
{

std::size_t Timer::GetWorker()
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
			LOG_WARN(Utils::GetLogger(), "Exception occured <Timer::LaunchOnce>: " + std::string(e.what()));
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
			auto pre = interval * dist(Utils::GetRngEngine());
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
				LOG_WARN(Utils::GetLogger(), "Exception occured <Timer::LaunchLoop>: " + std::string(e.what()));
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



std::size_t Timer::Launch(std::function<void()> func, std::function<time_t()> GetNext, int num)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = std::thread([this, func, GetNext, num, idx]
	{
		int count = 0;
		while (true)
		{
			time_t t = GetNext();
			{
				std::unique_lock<std::mutex> lk(this->mtx);
				if (cv.wait_until(lk, std::chrono::system_clock::from_time_t(t), [this, idx]{ return this->worker[idx].second.stop;}))
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
				LOG_WARN(Utils::GetLogger(), "Exception occured <Timer::Launch>: " + std::string(e.what()));
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