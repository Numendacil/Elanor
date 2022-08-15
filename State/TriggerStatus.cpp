#include <nlohmann/json.hpp>
#include "TriggerStatus.hpp"

using json = nlohmann::json;

namespace State
{

std::vector<std::string> TriggerStatus::GetTriggerList() const
{
	std::lock_guard<std::mutex> lk(this->mtx);
	std::vector<std::string> v;
	v.reserve(this->Enabled.size());
	for (const auto& p : this->Enabled)
		v.push_back(p.first);
	return v;
}

json TriggerStatus::Serialize()
{
	std::lock_guard<std::mutex> lk(this->mtx);
	using namespace std::chrono;
	json content;
	for (const auto& p : this->Enabled)
	{
		content[p.first] = p.second;
	}
	return content; 
}

void TriggerStatus::Deserialize(const json &content)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	for (const auto& p : content.items())
	{
			this->Enabled[p.key()] = p.value().get<bool>();

	}
}

}