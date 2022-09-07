
#include "Client.hpp"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>

#include <Utils/Logger.hpp>

#include <libmirai/Api/Client.hpp>

using namespace Mirai;

namespace
{

constexpr int INTERVAL = 500;
constexpr int MAX_RETRY = 3;

} // namespace

namespace Bot
{

Client::Client() : _connected(false), _interval(std::chrono::milliseconds(INTERVAL))
{
	this->_client = std::make_unique<MiraiClient>();
	this->_client->SetLogger(::Utils::GetLogger());
}
Client::~Client()
{
	if (this->_connected)
	{
		this->_connected = false;
		this->_cv.notify_one();
		if (this->_th.joinable()) this->_th.join();
		this->_client->Disconnect();
	}
}

void Client::MsgQueue()
{
	while (true)
	{
		Message msg;
		{
			std::unique_lock<std::mutex> lk(this->_mtx);
			this->_cv.wait(lk,
			               [this]() -> bool
			               {
							   if (!this->_connected) return true;
							   return !this->_messages.empty();
						   });
			if (!this->_connected) return;
			msg = std::move(this->_messages.front());
			this->_messages.pop();
		}

		try
		{
			std::this_thread::sleep_for(this->_interval);
			MessageId_t id = 0;
			switch (msg.type)
			{
			case Message::GROUP:
				id = this->_client->SendGroupMessage(msg.GroupId, msg.msg, msg.QuoteId, true);
				break;
			case Message::FRIEND:
				id = this->_client->SendFriendMessage(msg.qq, msg.msg, msg.QuoteId, true);
				break;
			case Message::TEMP:
				id = this->_client->SendTempMessage(msg.qq, msg.GroupId, msg.msg, msg.QuoteId, true);
				break;
			default:
				LOG_ERROR(*::Utils::GetLogger(), "waht");
			}
			msg.SendId.set_value(id);
		}
		catch (std::exception& e)
		{
			LOG_WARN(*::Utils::GetLogger(), std::string("MsgQueue: ") + e.what());
			if (msg.count + 1 < MAX_RETRY)
			{
				std::unique_lock<std::mutex> lk(this->_mtx);
				msg.count++;
				this->_messages.push(std::move(msg));
			}
		}
	}
}

std::future<Mirai::MessageId_t> Client::SendGroupMessage(GID_t GroupId, const MessageChain& msg,
                                                         std::optional<MessageId_t> QuoteId)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	auto SendId =
		this->_messages.emplace(GroupId, 0_qq, msg, QuoteId, Message::GROUP, std::promise<Mirai::MessageId_t>{}, 0)
			.SendId.get_future();
	this->_cv.notify_one();
	return SendId;
}

std::future<Mirai::MessageId_t> Client::SendFriendMessage(QQ_t qq, const MessageChain& msg,
                                                          std::optional<MessageId_t> QuoteId)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	auto SendId =
		this->_messages.emplace(0_gid, qq, msg, QuoteId, Message::FRIEND, std::promise<Mirai::MessageId_t>{}, 0)
			.SendId.get_future();
	this->_cv.notify_one();
	return SendId;
}

std::future<Mirai::MessageId_t> Client::SendTempMessage(GID_t GroupId, QQ_t qq, const MessageChain& msg,
                                                        std::optional<MessageId_t> QuoteId)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	auto SendId =
		this->_messages.emplace(GroupId, qq, msg, QuoteId, Message::TEMP, std::promise<Mirai::MessageId_t>{}, 0)
			.SendId.get_future();
	this->_cv.notify_one();
	return SendId;
}

void Client::Connect(const SessionConfigs& opts)
{
	this->_client->SetSessionConfig(opts);
	this->_client->Connect();
	this->_connected = true;
	this->_th = std::thread(&Client::MsgQueue, this);
}

void Client::Disconnect()
{
	this->_connected = false;
	this->_cv.notify_one();
	if (this->_th.joinable()) this->_th.join();
	this->_client->Disconnect();
}

} // namespace Bot