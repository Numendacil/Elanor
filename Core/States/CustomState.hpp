#ifndef _ELANOR_CORE_CUSTOM_STATE_HPP_
#define _ELANOR_CORE_CUSTOM_STATE_HPP_

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "StateBase.hpp"

namespace State
{

class CustomState : public StateBase
{
protected:
	mutable std::mutex _mtx;
	std::unordered_map<std::string, nlohmann::json> _states;

public:
	static constexpr std::string_view _NAME_ = "CustomState";

	nlohmann::json GetState(const std::string& id, const nlohmann::json& default_v = {});
	void SetState(const std::string& id, const nlohmann::json& content);
	void ModifyState(const std::string& id, std::function<void(nlohmann::json& content)> op,
	                 const nlohmann::json& default_v = {});

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& content) override;
};

} // namespace State

#endif