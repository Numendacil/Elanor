#ifndef _STATE_BASE_HPP_
#define _STATE_BASE_HPP_

#include <string>
#include <nlohmann/json.hpp>

namespace State
{

class StateBase
{
public:
	// Return empty string if no serialization needed
	virtual nlohmann::json Serialize() { return nlohmann::json(); }
	virtual void Deserialize(const nlohmann::json&) {}

	virtual ~StateBase() = default;
};

}

#endif