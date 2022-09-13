#ifndef _ELANOR_CORE_STATE_BASE_HPP_
#define _ELANOR_CORE_STATE_BASE_HPP_

#include <string>

#include <nlohmann/json_fwd.hpp>

namespace State
{

class StateBase
{
public:
	// Return empty json if no serialization needed
	virtual nlohmann::json Serialize();
	virtual void Deserialize(const nlohmann::json&);

	virtual ~StateBase() = default;
};

} // namespace State

#endif