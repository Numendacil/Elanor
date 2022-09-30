#ifndef _BILILIVE_TRIGGER_
#define _BILILIVE_TRIGGER_

#include <string>

#include <Core/Interface/ITrigger.hpp>

namespace Trigger
{

class BililiveTrigger : public ITrigger
{
public:
	static constexpr std::string_view _NAME_ = "Bililive";
	
	void Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config) override;
	time_t GetNext() override;
	bool isDefaultOn() const override { return false; }
};

}

#endif