#ifndef _COOLDOWN_HPP_
#define _COOLDOWN_HPP_

#include <mutex>
#include <string>
#include <sstream>
#include <unordered_map>
#include <optional>
#include <chrono>

#include <State/StateBase.hpp>

namespace State
{

class CoolDown : public StateBase
{
protected:
	mutable std::mutex mtx;
	
	struct command_state
	{
		bool isUsing = false;
		std::optional<std::chrono::system_clock::time_point> LastUsed = std::nullopt;
	};

	std::unordered_map<std::string, command_state> cd;

public:

	static constexpr std::string_view _NAME_ = "CoolDown";

	struct Holder
	{
	private:
		CoolDown* obj;
		std::string id;
	public:
		Holder(CoolDown* cd, const std::string& str) : obj(cd), id(str) {}
		Holder(Holder&) = delete;
		~Holder()
		{
			std::lock_guard<std::mutex> lk(obj->mtx);
			obj->cd[id].isUsing = false;
			obj->cd[id].LastUsed = std::chrono::system_clock::now();
		}
	};

	std::unique_ptr<Holder> GetRemaining(const std::string& id, const std::chrono::seconds& cd, std::chrono::seconds& remaining);

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;

};

}

#endif