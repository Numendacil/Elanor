#ifndef _BILILIVE_TRIGGER_
#define _BILILIVE_TRIGGER_

#include <queue>
#include <string>
#include <set>

#include <libmirai/models.hpp>

#include <Core/Interface/ITrigger.hpp>

namespace Trigger
{

class BililiveTrigger : public ITrigger
{
protected:
	struct Stream
	{
		uint64_t RoomId{};
		uint64_t uid{};
		std::set<Mirai::GID_t> groups;
	};

	std::queue<Stream> _streams;

public:
	static constexpr std::string_view _NAME_ = "Bililive";

	void Action(Bot::GroupList& groups, Bot::Client& client, Utils::BotConfig& config) override;
	time_t GetNext() override;
	bool isDefaultOn() const override { return false; }
};

} // namespace Trigger

#endif