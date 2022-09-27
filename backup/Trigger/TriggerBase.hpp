#ifndef _TRIGGER_BASE_HPP_
#define _TRIGGER_BASE_HPP_

#include <string>
#include <memory>
#include <unordered_map>

namespace Bot
{

class GroupList;

}

namespace Trigger
{

class TriggerBase
{
protected:
	bool is_running;

	std::weak_ptr<Bot::GroupList> groups;

public:
	TriggerBase() : is_running(false) {}
	virtual bool TriggerOnStart() { return false; }
	virtual void TriggerOn()
	{
		this->is_running = true;
	}

	virtual void TriggerOff()
	{
		this->is_running = false;
	}

	virtual bool IsRunning()
	{
		return this->is_running;
	}
	
	virtual ~TriggerBase() 
	{ 
		this->is_running = false;
	}

	void SetGroups(const std::shared_ptr<Bot::GroupList>& glist)
	{
		this->groups = glist;
	}
};

}

#endif