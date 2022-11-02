#include "Group.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>

#include <nlohmann/json.hpp>

#include <Core/States/States.hpp>
#include <Core/Utils/Logger.hpp>


using json = nlohmann::json;
using std::string;

namespace Bot
{

namespace
{

std::unordered_map<string, std::unique_ptr<State::StateBase>> RegisterStates()
{
	std::unordered_map<string, std::unique_ptr<State::StateBase>> v{};

#define REGISTER(_class_) v[string(_class_::_NAME_)] = std::make_unique<_class_>()

	REGISTER(State::AccessCtrlList);
	REGISTER(State::Activity);
	REGISTER(State::CommandPerm);
	REGISTER(State::CoolDown);
	REGISTER(State::CustomState);
	REGISTER(State::TriggerStatus);

#undef REGISTER

	return v;
}

} // namespace


Group::Group(Mirai::GID_t group_id) : gid(group_id), _states(RegisterStates()) {}

void Group::ToFile(const std::filesystem::path& filepath) const
{
	json content{};
	{
		std::lock_guard<std::mutex> lk(this->_mtx_state);
		for (const auto& p : this->_states)
			if (!p.second->Serialize().empty()) content["States"][p.first] = p.second->Serialize();
	}

	try
	{
		std::filesystem::create_directories(filepath.parent_path());
	}
	catch (const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(),
		         "Failed to create directory " + string(filepath.parent_path()) + ": " + std::string(e.what()));
		return;
	}

	{
		std::lock_guard<std::mutex> lk(this->_mtx_file);
		std::ofstream file(filepath);
		if (!file)
		{
			LOG_WARN(Utils::GetLogger(), "Failed to open file " + string(filepath) + " for writing");
			return;
		}
		
		LOG_DEBUG(Utils::GetLogger(), "Writing to file " + string(filepath));
		file << content.dump(1, '\t');
	}
}

void Group::FromFile(const std::filesystem::path& filepath)
{
	json content;
	if (!std::filesystem::exists(filepath)) return;
	try
	{
		std::lock_guard<std::mutex> lk(this->_mtx_file);
		std::ifstream file(filepath);
		if (!file)
		{
			LOG_WARN(Utils::GetLogger(), "Failed to open file " + string(filepath) + " for reading");
			return;
		}
		content = json::parse(file);
	}
	catch (json::parse_error& e)
	{
		LOG_WARN(Utils::GetLogger(), "Failed to parse file " + string(filepath) + " :" + e.what());
		return;
	}

	LOG_DEBUG(Utils::GetLogger(), "Reading from file " + string(filepath));
	{
		std::lock_guard<std::mutex> lk(this->_mtx_state);
		if (content.contains("States"))
		{
			assert(content["States"].type() == json::value_t::object);
			for (const auto& p : content["States"].items())
				if (this->_states.count(p.key())) this->_states.at(p.key())->Deserialize(p.value());
		}
	}
}

} // namespace Bot