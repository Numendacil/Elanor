
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>

#include <ThirdParty/log.h>
#include <libmirai/Api/Client.hpp>

#include "Client.hpp"

using namespace Mirai;

namespace Bot
{

Client::Client()
{
	this->Connected = false;
	this->interval = std::chrono::milliseconds(500);
	this->max_retry = 3;
	this->client = std::make_unique<MiraiClient>();
}
Client::~Client()
{
	if (this->Connected)
	{
		this->Connected = false;
		this->cv.notify_one();
		if (this->th.joinable())
			this->th.join();
		this->client->Disconnect();
	}
}

void Client::MsgQueue()
{
	while(true)
	{
		Message msg;
		{
			std::unique_lock<std::mutex> lk(this->q_mtx);
			this->cv.wait(lk, [this]() -> bool
			{ 
				if (!this->Connected)
					return true;
				return !this->message.empty();
			});
			if (!this->Connected)
				return;
			msg = std::move(this->message.front());
			this->message.pop();
		}

		try
		{
			std::this_thread::sleep_for(this->interval);
			MessageId_t id = 0;
			switch (msg.type)
			{
			case Message::GROUP:
				id = this->client->SendGroupMessage(msg.GroupId, msg.msg, msg.QuoteId, true);
				break;
			case Message::FRIEND:
				id = this->client->SendFriendMessage(msg.qq, msg.msg, msg.QuoteId, true);
				break;
			case Message::TEMP:
				id = this->client->SendTempMessage(msg.qq, msg.GroupId, msg.msg, msg.QuoteId, true);
				break;
			default:
				logging::ERROR("waht");
			}
			msg.SendId.set_value(id);
		}
		catch(std::exception& e)
		{
			logging::WARN(std::string("MsgQueue: ") + e.what());
			if (msg.count + 1 < this->max_retry)
			{
				std::unique_lock<std::mutex> lk(this->q_mtx);
				msg.count++;
				this->message.push(std::move(msg));
			}
		}
	}
}

std::future<Mirai::MessageId_t> Client::SendGroupMessage(GID_t GroupId, const MessageChain& msg, std::optional<MessageId_t> QuoteId)
{
	std::unique_lock<std::mutex> lk(this->q_mtx);
	auto SendId = this->message.emplace(GroupId, 0_qq, msg, QuoteId, Message::GROUP, std::promise<Mirai::MessageId_t>{}, 0).SendId.get_future();
	this->cv.notify_one();
	return SendId;
}

std::future<Mirai::MessageId_t> Client::SendFriendMessage(QQ_t qq, const MessageChain& msg, std::optional<MessageId_t> QuoteId)
{
	std::unique_lock<std::mutex> lk(this->q_mtx);
	auto SendId = this->message.emplace(0_gid, qq, msg, QuoteId, Message::FRIEND, std::promise<Mirai::MessageId_t>{}, 0).SendId.get_future();
	this->cv.notify_one();
	return SendId;
}

std::future<Mirai::MessageId_t> Client::SendTempMessage(GID_t GroupId, QQ_t qq, const MessageChain& msg, std::optional<MessageId_t> QuoteId)
{
	std::unique_lock<std::mutex> lk(this->q_mtx);
	auto SendId = this->message.emplace(GroupId, qq, msg, QuoteId, Message::TEMP, std::promise<Mirai::MessageId_t>{}, 0).SendId.get_future();
	this->cv.notify_one();
	return SendId;
}

void Client::Connect(const SessionConfigs &opts)
{
	this->client->SetSessionConfig(opts);
	this->client->Connect();
	this->Connected = true;
	this->th = std::thread(&Client::MsgQueue, this);
}

void Client::Disconnect()
{
	this->Connected = false;
	this->cv.notify_one();
	if (this->th.joinable())
		this->th.join();
	this->client->Disconnect();
}

}