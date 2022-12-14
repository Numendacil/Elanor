#ifndef _SEKAI_UPDATE_TRIGGER_
#define _SEKAI_UPDATE_TRIGGER_

#include <string>

#include <Core/Interface/ITrigger.hpp>

namespace Trigger
{

class SekaiUpdateTrigger : public ITrigger
{
public:
	static constexpr std::string_view _NAME_ = "SekaiUpdate";

	void Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config) override;
	time_t GetNext() override;
	bool isDefaultOn() const override { return false; }
};

} // namespace Trigger

#endif