#ifndef _MORNING_TRIGGER_HPP_
#define _MORNING_TRIGGER_HPP_

#include <string_view>

#include <Core/Interface/ITrigger.hpp>

namespace Trigger
{

class MorningTrigger : public ITrigger
{
public:
	void Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config) override;
	time_t GetNext() override;
	bool isDefaultOn() const override { return true; }

	static constexpr std::string_view _NAME_ = "Morning";
};

} // namespace Trigger

#endif