#ifndef _ELANOR_CORE_COMMAND_PERMISSION_HPP_
#define _ELANOR_CORE_COMMAND_PERMISSION_HPP_

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cassert>

#include "StateBase.hpp"

namespace State
{

class CommandPerm : public StateBase
{
protected:
	mutable std::mutex _mtx;

	std::unordered_map<std::string, std::pair<int, int>> _permissions; // { key : {current, default} }

public:
	static constexpr std::string_view _NAME_ = "CommandPerm";

	bool ExistCommand(const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		return this->_permissions.count(command);
	}

	std::pair<int, int> GetPermission(const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		assert(this->_permissions.count(command));
		return this->_permissions.at(command);
	}

	bool UpdatePermission(const std::string& command, int perm)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		if (!this->_permissions.count(command)) return false;
		this->_permissions[command].first = perm;
		return true;
	}

	void AddCommand(const std::string& command, int perm)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_permissions[command].first = perm;
		this->_permissions[command].second = perm;
	}

	std::vector<std::string> GetCommandList() const;


	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& content) override;
};

} // namespace State

#endif