#ifndef _ELANOR_CORE_TRIGGER_STATUS_HPP_
#define _ELANOR_CORE_TRIGGER_STATUS_HPP_

#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>

#include "StateBase.hpp"

namespace State
{

class TriggerStatus : public StateBase
{
protected:
	mutable std::mutex _mtx;

	std::unordered_map<std::string, bool> _enabled; // { key : {current, default} }

public:
	static constexpr std::string_view _NAME_ = "TriggerStatus";

	bool ExistTrigger(const std::string& trigger) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		return this->_enabled.count(trigger);
	}

	bool GetTriggerStatus(const std::string& trigger) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		assert(this->_enabled.count(trigger));
		return this->_enabled.at(trigger);
	}

	bool UpdateTriggerStatus(const std::string& trigger, bool status)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		if (!this->_enabled.count(trigger)) return false;
		this->_enabled[trigger] = status;
		return true;
	}

	void AddTrigger(const std::string& trigger, bool status)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_enabled[trigger] = status;
	}

	std::vector<std::string> GetTriggerList() const;

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& content) override;
};

} // namespace State

#endif