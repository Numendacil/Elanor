#include <map>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace State
{

struct BililiveList
{
	static constexpr std::string_view _NAME_ = "BililiveList";
	struct info
	{
		uint64_t room_id = 0;
		bool broadcasted = false;
	};
	std::map<uint64_t, info> user_list; // Map uid -> room_id
};

void from_json(const nlohmann::json& j, BililiveList& p);

void to_json(nlohmann::json& j, const BililiveList& p);

} // namespace State