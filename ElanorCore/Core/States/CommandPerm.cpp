#include "CommandPerm.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace State
{

std::vector<std::string> CommandPerm::GetCommandList() const
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	std::vector<std::string> v;
	v.reserve(this->_permissions.size());
	for (const auto& p : this->_permissions)
		v.push_back(p.first);
	return v;
}

json CommandPerm::Serialize()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	using namespace std::chrono;
	json content;
	for (const auto& p : this->_permissions)
	{
		content[p.first]["current"] = p.second.first;
		content[p.first]["default"] = p.second.second;
	}
	return content;
}

void CommandPerm::Deserialize(const json& content)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	for (const auto& p : content.items())
	{
		if (p.value().contains("current")) this->_permissions[p.key()].first = p.value().at("current").get<int>();
		// if (p.value().contains("default")) this->_permissions[p.key()].second = p.value()["default"].get<int>();
	}
}

} // namespace State