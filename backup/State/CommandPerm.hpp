#ifndef _COMMAND_PERMISSION_HPP_
#define _COMMAND_PERMISSION_HPP_

#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <State/StateBase.hpp>

namespace State
{

class CommandPerm : public StateBase
{
protected:
	mutable std::mutex mtx;
	
	std::unordered_map<std::string, std::pair<int, int>> Permission;	// { key : {current, default} }

public:

	static constexpr std::string_view _NAME_ = "CommandPerm";

	bool ExistCommand(const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->Permission.count(command);
	}

	std::pair<int, int> GetPermission(const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->Permission.count(command));
		return this->Permission.at(command);
	}

	bool UpdatePermission(const std::string& command, int perm)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->Permission.count(command))
			return false;
		this->Permission[command].first = perm;
		return true;
	}

	void AddCommand(const std::string& command, int perm)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->Permission[command].first = perm;
		this->Permission[command].second = perm;
	}

	std::vector<std::string> GetCommandList() const;
	

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;

};

}

#endif