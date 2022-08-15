#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <functional>

namespace Utils
{

class Timer
{
public:
	Timer(const Timer&) = delete;
	void operator=(const Timer&) = delete;

	static Timer& GetInstance(void)
	{
		static Timer t;
		return t;
	}

	std::size_t LaunchOnce(std::function<void()> func, std::chrono::milliseconds delay);
	std::size_t LaunchLoop(std::function<void()> func, std::chrono::milliseconds interval, bool RandStart = false);
	std::size_t LaunchAt(std::function<void()> func, const std::string& cron_str, int num = -1);

	void Stop(size_t id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		for (auto& p : this->worker)
		{
			if (p.second.id == id)
				p.second.stop = true;
		}
		cv.notify_all();
	}
	
	bool IsRunning(size_t id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		for (auto& p : this->worker)
		{
			if (p.second.id == id)
				return !p.second.finished;
		}
		return false;
	}


	~Timer()
	{
		for (auto& p : this->worker)
			p.second.stop = true;
		cv.notify_all();
		for (auto& p : this->worker)
			if (p.first.joinable())
				p.first.join();
	}

private:
	Timer() {};
	std::size_t GetWorker(void);

	struct WorkerState
	{
		std::size_t id;
		bool finished = true;
		bool stop = false;
	};

	std::vector<std::pair<std::thread, WorkerState>> worker;
	std::mutex mtx;
	std::condition_variable cv;
	std::size_t id_count = 0;
};

}

#endif