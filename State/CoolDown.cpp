#include <nlohmann/json.hpp>
#include <ThirdParty/date.h>
#include "CoolDown.hpp"

using json = nlohmann::json;

namespace State
{

std::unique_ptr<CoolDown::Holder> CoolDown::GetRemaining(const std::string &id, const std::chrono::seconds &cd, std::chrono::seconds &remaining)
{
	using namespace date;
	using namespace std::chrono;
	std::lock_guard<std::mutex> lk(this->mtx);
	if (this->cd[id].isUsing)
	{
		remaining = cd;
		return nullptr;
	}

	if (!this->cd[id].LastUsed.has_value())
	{
		this->cd[id].isUsing = true;
		remaining = 0s;
		return std::make_unique<Holder>(this, id);
	}
	
	auto passed = floor<seconds>(system_clock::now() - this->cd[id].LastUsed.value());
	if (passed >= cd)
	{
		this->cd[id].isUsing = true;
		remaining = 0s;
		return std::make_unique<Holder>(this, id);
	}
	else
	{
		remaining = cd - passed;
		return nullptr;
	}

}

json CoolDown::Serialize()
{
	std::lock_guard<std::mutex> lk(this->mtx);
	using namespace std::chrono;
	json content;
	for (const auto& p : this->cd)
	{
		if (p.second.LastUsed.has_value())
		{
			auto t = time_point_cast<seconds>(*p.second.LastUsed).time_since_epoch();
			content[p.first] = t.count();
		}
	}
	return content; 
}

void CoolDown::Deserialize(const json &content)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	for (const auto& p : content.items())
	{
		using namespace std::chrono;
		command_state state;
		state.isUsing = false;
		state.LastUsed = time_point<system_clock>(seconds(p.value()));
		this->cd[p.key()] = state;
	}
}

}