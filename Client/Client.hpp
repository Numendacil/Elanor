#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

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

#include <libmirai/Types/BasicTypes.hpp>
#include <libmirai/Messages/MessageChain.hpp>

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
	std::unique_ptr<Mirai::MiraiClient> client;
	
	mutable std::mutex q_mtx;
	mutable std::mutex client_mtx;
	mutable std::condition_variable cv;
	std::thread th;

	bool Connected;

	struct Message
	{
		Mirai::GID_t GroupId;
		Mirai::QQ_t qq;
		Mirai::MessageChain msg;
		std::optional<Mirai::MessageId_t> QuoteId = std::nullopt;
		enum {GROUP, FRIEND, TEMP} type = GROUP;

		std::promise<Mirai::MessageId_t> SendId;

		int count = 0;
	};

	std::queue<Message> message;
	std::chrono::milliseconds interval;
	int max_retry;

	Client();
	void MsgQueue();

public:
	Client(const Client&) = delete;
	~Client();

	static Client& GetClient()
	{
		static Client client;
		return client;
	}

	Mirai::MiraiClient& GetMiraiClient()
	{
		return *client;
	}

	// template <typename T>
	// void On(const std::function<void(T)>& ep)
	// {
	// 	return this->client->On<T>(ep);
	// }

	void Connect(const Mirai::SessionConfigs& opts);
	void Reconnect();
	void Disconnect();
	bool isConnected() { return this->Connected; }

	// template<typename F, typename... Args>
	// auto Call(F&& f, Args&&... args)
	// {
	// 	std::lock_guard<std::mutex> lk(this->client_mtx);
	// 	return std::invoke(std::forward<F>(f), this->client, std::forward<Args>(args)...);
	// }

	std::future<Mirai::MessageId_t> SendGroupMessage(Mirai::GID_t, const Mirai::MessageChain&, std::optional<Mirai::MessageId_t> = std::nullopt);
	std::future<Mirai::MessageId_t> SendFriendMessage(Mirai::QQ_t, const Mirai::MessageChain&, std::optional<Mirai::MessageId_t> = std::nullopt);
	std::future<Mirai::MessageId_t> SendTempMessage(Mirai::GID_t, Mirai::QQ_t, const Mirai::MessageChain&, std::optional<Mirai::MessageId_t> = std::nullopt);

};

}

#endif