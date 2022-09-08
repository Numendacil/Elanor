#include "Group.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>

#include <Core/States/States.hpp>
#include <Core/Utils/Logger.hpp>
#include <nlohmann/json.hpp>


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


Group::Group(Mirai::GID_t group_id, Mirai::QQ_t owner_id, const std::vector<std::pair<std::string, int>>& command_list,
             const std::vector<std::pair<std::string, bool>>& trigger_list)
	: gid(group_id), suid(owner_id), _states(RegisterStates())
{
	auto command = this->GetState<State::CommandPerm>();
	for (const auto& p : command_list)
		command->AddCommand(p.first, p.second);

	auto trigger = this->GetState<State::TriggerStatus>();
	for (const auto& p : trigger_list)
		trigger->AddTrigger(p.first, p.second);

	this->FromFile();
}

void Group::ToFile()
{
	json content{};
	{
		std::lock_guard<std::mutex> lk(this->_mtx_state);
		for (const auto& p : this->_states)
			if (!p.second->Serialize().empty()) content["States"][p.first] = p.second->Serialize();
	}

	std::filesystem::path Path = "./bot";
	try
	{
		std::filesystem::create_directory(Path);
	}
	catch (const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(), "Failed to create directory ./bot/: " + std::string(e.what()));
		return;
	}

	Path /= this->gid.to_string();
	{
		std::lock_guard<std::mutex> lk(this->_mtx_file);
		std::ofstream file(Path);
		if (!file)
		{
			LOG_WARN(Utils::GetLogger(), "Failed to open file " + string(Path) + " for writing");
			return;
		}
		LOG_INFO(Utils::GetLogger(), "Writing to file " + string(Path));
		file << content.dump(1, '\t');
	}
}

void Group::FromFile()
{
	json content;
	std::filesystem::path Path = "./bot";
	Path /= this->gid.to_string();
	if (!std::filesystem::exists(Path)) return;
	try
	{
		std::lock_guard<std::mutex> lk(this->_mtx_file);
		std::ifstream file(Path);
		if (!file)
		{
			LOG_WARN(Utils::GetLogger(), "Failed to open file " + string(Path) + " for reading");
			return;
		}
		content = json::parse(file);
	}
	catch (json::parse_error& e)
	{
		LOG_WARN(Utils::GetLogger(), "Failed to parse file " + string(Path) + " :" + e.what());
		return;
	}
	LOG_INFO(Utils::GetLogger(), "Reading from file " + string(Path));
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