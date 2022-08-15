#ifndef _TRIGGER_STATUS_HPP_
#define _TRIGGER_STATUS_HPP_

#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>

#include <State/StateBase.hpp>

namespace State
{

class TriggerStatus : public StateBase
{
protected:
	mutable std::mutex mtx;
	
	std::unordered_map<std::string, bool> Enabled;	// { key : {current, default} }

public:

	static constexpr std::string_view _NAME_ = "TriggerStatus";

	bool ExistTrigger(const std::string& trigger) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->Enabled.count(trigger);
	}

	bool GetTriggerStatus(const std::string& trigger) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->Enabled.count(trigger));
		return this->Enabled.at(trigger);
	}

	bool UpdateTriggerStatus(const std::string& trigger, bool status)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->Enabled.count(trigger))
			return false;
		this->Enabled[trigger] = status;
		return true;
	}

	void AddTrigger(const std::string& trigger, bool status)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->Enabled[trigger] = status;
	}
	
	std::vector<std::string> GetTriggerList() const;

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;

};

}

#endif