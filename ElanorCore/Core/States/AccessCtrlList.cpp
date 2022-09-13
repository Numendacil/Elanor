#include "AccessCtrlList.hpp"

#include <nlohmann/json.hpp>
#include "libmirai/Types/BasicTypes.hpp"

using json = nlohmann::json;

namespace State
{

json AccessCtrlList::Serialize()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	json content;
	for (const auto& p : this->_WhiteList)
		content["WhiteList"] += (int64_t)p;
	for (const auto& p : this->_BlackList)
		content["BlackList"] += (int64_t)p;
	content["suid"] = this->_suid;
	return content;
}

void AccessCtrlList::Deserialize(const json& content)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	if (content.contains("WhiteList"))
	{
		if (content.at("WhiteList").type() == json::value_t::array)
		{
			for (const auto& p : content.at("WhiteList"))
				this->_WhiteList.insert(p.get<Mirai::QQ_t>());
		}
	}
	if (content.contains("BlackList"))
	{
		if (content.at("BlackList").type() == json::value_t::array)
		{
			for (const auto& p : content.at("BlackList"))
				this->_BlackList.insert(p.get<Mirai::QQ_t>());
		}
	}
	if (content.contains("suid"))
	{
		this->_suid = content.at("suid").get<Mirai::QQ_t>();
	}
}

} // namespace State