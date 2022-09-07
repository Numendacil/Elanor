#include "CustomState.hpp"

#include <mutex>

using json = nlohmann::json;

namespace State
{

json CustomState::GetState(const std::string& id, const json& default_v)
{
	std::lock_guard<std::mutex> lk(this->_mtx);

	auto p = this->_states.try_emplace(id, default_v);
	return p.first->second;
}

void CustomState::SetState(const std::string& id, const json& content)
{
	std::lock_guard<std::mutex> lk(this->_mtx);

	this->_states[id] = content;
}

void CustomState::ModifyState(const std::string& id, std::function<void(json&)> op, const json& default_v)
{
	std::lock_guard<std::mutex> lk(this->_mtx);

	auto p = this->_states.try_emplace(id, default_v);
	op(p.first->second);
}

json CustomState::Serialize()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	json content;
	for (const auto& p : this->_states)
	{
		json data;
		data["id"] = p.first;
		data["state"] = p.second;
		content += data;
	}
	return content;
}

void CustomState::Deserialize(const json& content)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	assert(content.is_array());
	for (const auto& p : content)
	{
		this->_states.try_emplace(p.at("id").get<std::string>(), p.at("state"));
	}
}

} // namespace State