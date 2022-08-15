#include <nlohmann/json.hpp>
#include <libmirai/Messages/MessageChain.hpp>
#include "LastMessage.hpp"

using json = nlohmann::json;

namespace State
{

void LastMessage::Set(const Mirai::MessageChain &m, bool r)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	this->LastMsg = m;
	this->Repeated = r;
}

void LastMessage::Get(Mirai::MessageChain &m, bool &r) const
{
	std::lock_guard<std::mutex> lk(this->mtx);
	m = this->LastMsg;
	r = this->Repeated;
}

json LastMessage::Serialize()
{
	std::lock_guard<std::mutex> lk(this->mtx);
	json content;
	content["Message"] = this->LastMsg.ToJson();
	content["Repeated"] = this->Repeated;
	return content; 
}

void LastMessage::Deserialize(const json &content)
{ 
	std::lock_guard<std::mutex> lk(this->mtx);
	if (content.contains("Message"))
	{
		this->LastMsg = content["Message"].get<Mirai::MessageChain>();
	}
	if (content.contains("Repeated"))
		this->Repeated = content["Repeated"].get<bool>();
}

}