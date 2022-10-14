#include "BililiveList.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace State
{

void from_json(const json& j, BililiveList& p)
{
	for (const auto& data : j)
	{
		uint64_t room_id = data.value("room_id", 0);
		bool broadcasted = data.value("broadcasted", false);
		uint64_t uid = data.value("uid", 0);
		p.user_list.emplace(uid, BililiveList::info{room_id, broadcasted});
	}
}

void to_json(json& j, const BililiveList& p)
{
	j = {};
	for (const auto& u : p.user_list)
	{
		json member{};
		member["uid"] = u.first;
		member["room_id"] = u.second.room_id;
		member["broadcasted"] = u.second.broadcasted;
		j += member;
	}
}

} // namespace State