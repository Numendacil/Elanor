#ifndef _ELANOR_CORE_CLIENT_HPP_
#define _ELANOR_CORE_CLIENT_HPP_

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

#include <libmirai/Messages/MessageChain.hpp>
#include <libmirai/Types/BasicTypes.hpp>

namespace Mirai
{

class MiraiClient;
class SessionConfigs;

}

namespace Bot
{

class Client
{
protected:
	std::unique_ptr<Mirai::MiraiClient> _client;

	mutable std::mutex _mtx;
	mutable std::condition_variable _cv;
	std::thread _th;

	bool _connected;
	bool _paused;

	struct Message
	{
		Mirai::GID_t GroupId;
		Mirai::QQ_t qq;
		Mirai::MessageChain msg;
		std::optional<Mirai::MessageId_t> QuoteId = std::nullopt;
		enum
		{
			GROUP,
			FRIEND,
			TEMP
		} type = GROUP;

		std::promise<Mirai::MessageId_t> SendId;

		int count = 0;
	};

	std::queue<Message> _messages;
	const std::chrono::milliseconds _interval;

	void MsgQueue();

public:
	Client();

	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(Client&&) = delete;

	~Client();

	Mirai::MiraiClient* operator->()
	{
		std::unique_lock<std::mutex> lk(this->_mtx);
		this->_cv.wait(lk, [this]() -> bool {
			if (!this->_connected) return true;
			return !_paused;
		});
		if (!this->_connected)
			throw std::runtime_error("Connection Lost");
		return this->_client.get();
	}

	void Connect(const Mirai::SessionConfigs& opts);
	void Disconnect();
	bool isConnected() 
	{ 
		std::lock_guard<std::mutex> lk(this->_mtx);
		return this->_connected; 
	}

	void Pause()
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_paused = true;
		this->_cv.notify_all();
	}
	void Resume()
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_paused = false;
		this->_cv.notify_all();
	}
	bool isRunning()
	{ 
		std::lock_guard<std::mutex> lk(this->_mtx);
		return !this->_paused; 
	}

	// template<typename F, typename... Args>
	// auto Call(F&& f, Args&&... args)
	// {
	// 	std::unique_lock<std::mutex> lk(this->_mtx);
	// 	this->_cv.wait(lk, [this]() -> bool {
	// 		if (!this->_connected) return true;
	// 		return !_paused;
	// 	});
	// 	if (!this->_connected)
	// 		throw std::runtime_error("Connection Lost");
	// 	return std::invoke(std::forward<F>(f), this->_client, std::forward<Args>(args)...);
	// }

	std::future<Mirai::MessageId_t> SendGroupMessage(Mirai::GID_t, const Mirai::MessageChain&,
	                                                 std::optional<Mirai::MessageId_t> = std::nullopt);
	std::future<Mirai::MessageId_t> SendFriendMessage(Mirai::QQ_t, const Mirai::MessageChain&,
	                                                  std::optional<Mirai::MessageId_t> = std::nullopt);
	std::future<Mirai::MessageId_t> SendTempMessage(Mirai::GID_t, Mirai::QQ_t, const Mirai::MessageChain&,
	                                                std::optional<Mirai::MessageId_t> = std::nullopt);
};

} // namespace Bot

#endif