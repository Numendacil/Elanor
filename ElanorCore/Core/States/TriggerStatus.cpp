#include "TriggerStatus.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace State
{

std::vector<std::string> TriggerStatus::GetTriggerList() const
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	std::vector<std::string> v;
	v.reserve(this->_enabled.size());
	for (const auto& p : this->_enabled)
		v.push_back(p.first);
	return v;
}

json TriggerStatus::Serialize()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	using namespace std::chrono;
	json content;
	for (const auto& p : this->_enabled)
	{
		content[p.first] = p.second;
	}
	return content;
}

void TriggerStatus::Deserialize(const json& content)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	for (const auto& p : content.items())
	{
		this->_enabled[p.key()] = p.value().get<bool>();
	}
}

} // namespace State