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
#include <thread>
#include <utility>

#include <libmirai/Messages/MessageChain.hpp>
#include <libmirai/Types/BasicTypes.hpp>

namespace Mirai
{

class MiraiClient;
class SessionConfigs;

} // namespace Mirai

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

	Mirai::MiraiClient& GetMiraiClient() { return *(this->_client); }

	// template <typename T>
	// void On(const std::function<void(T)>& ep)
	// {
	// 	return this->client->On<T>(ep);
	// }

	void Connect(const Mirai::SessionConfigs& opts);
	void Reconnect();
	void Disconnect();
	bool isConnected() { return this->_connected; }

	// template<typename F, typename... Args>
	// auto Call(F&& f, Args&&... args)
	// {
	// 	std::lock_guard<std::mutex> lk(this->client_mtx);
	// 	return std::invoke(std::forward<F>(f), this->client, std::forward<Args>(args)...);
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