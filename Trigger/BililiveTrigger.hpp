#ifndef _BILILIVE_TRIGGER_
#define _BILILIVE_TRIGGER_

#include <Trigger/LoopTriggerBase.hpp>
#include <string_view>

namespace Trigger
{

class BililiveTrigger : public LoopTriggerBase
{
protected:
	virtual void Action() override;

	virtual std::chrono::milliseconds LoopInterval() override { return std::chrono::minutes(5); }
	virtual bool RandStart() override { return true; }

public:

	static constexpr std::string_view _NAME_ = "Bililive";

};

}

#endif