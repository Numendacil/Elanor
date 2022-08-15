#include <nlohmann/json.hpp>
#include "AccessCtrlList.hpp"

using json = nlohmann::json;

namespace State
{

json AccessCtrlList::Serialize()
{
	std::lock_guard<std::mutex> lk(this->mtx);
	json content;
	for (const auto& p : this->WhiteList)
		content["WhiteList"] += (int64_t)p;
	for (const auto& p : this->BlackList)
		content["BlackList"] += (int64_t)p;
	return content;
}

void AccessCtrlList::Deserialize(const json &content)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	if (content.contains("WhiteList"))
	{
		if (content["WhiteList"].type() == json::value_t::array)
		{
			for (const auto& p : content["WhiteList"])
				this->WhiteList.insert(p.get<Mirai::QQ_t>());
		}
	}
	if (content.contains("BlackList"))
	{
		if (content["BlackList"].type() == json::value_t::array)
		{
			for (const auto& p : content["BlackList"])
				this->BlackList.insert(p.get<Mirai::QQ_t>());
		}
	}
}

}