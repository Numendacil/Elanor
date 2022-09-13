#include "StateBase.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace State
{

json StateBase::Serialize()
{
	return {};
}

void StateBase::Deserialize(const json&) {}

} // namespace State