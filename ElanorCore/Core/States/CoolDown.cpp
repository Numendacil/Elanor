#include "CoolDown.hpp"

#include <chrono>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace State
{

std::unique_ptr<CoolDown::Token> CoolDown::GetRemaining(const std::string& id, const std::chrono::seconds& _cd,
                                                        std::chrono::seconds& remaining)
{
	using namespace std::chrono;
	std::lock_guard<std::mutex> lk(this->_mtx);
	if (this->_cd[id].isUsing)
	{
		remaining = _cd;
		return nullptr;
	}

	if (!this->_cd[id].LastUsed.has_value())
	{
		this->_cd[id].isUsing = true;
		remaining = 0s;
		return std::make_unique<Token>(this, id);
	}

	auto passed = floor<seconds>(system_clock::now() - this->_cd[id].LastUsed.value());
	if (passed >= _cd)
	{
		this->_cd[id].isUsing = true;
		remaining = 0s;
		return std::make_unique<Token>(this, id);
	}
	else
	{
		remaining = _cd - passed;
		return nullptr;
	}
}

json CoolDown::Serialize()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	using namespace std::chrono;
	json content;
	for (const auto& p : this->_cd)
	{
		if (p.second.LastUsed.has_value())
		{
			auto t = time_point_cast<seconds>(*p.second.LastUsed).time_since_epoch();
			content[p.first] = t.count();
		}
	}
	return content;
}

void CoolDown::Deserialize(const json& content)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	for (const auto& p : content.items())
	{
		using namespace std::chrono;
		command_state state;
		state.isUsing = false;
		state.LastUsed = time_point<system_clock>(seconds(p.value()));
		this->_cd[p.key()] = state;
	}
}

} // namespace State