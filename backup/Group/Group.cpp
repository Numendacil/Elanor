#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>

#include <nlohmann/json.hpp>
#include <State/State.hpp>
#include <ThirdParty/log.h>

#include <Utils/Timer.hpp>

#include "Group.hpp"


using json = nlohmann::json;
using std::string;

namespace Bot
{

std::unordered_map<string, std::unique_ptr<State::StateBase>> RegisterStates()
{
	std::unordered_map<string, std::unique_ptr<State::StateBase>> v;

	#define REGISTER(_class_) v[string(_class_::_NAME_)] = std::make_unique<_class_>();

	REGISTER(State::AccessCtrlList)
	REGISTER(State::Activity)
	REGISTER(State::BililiveList)
	REGISTER(State::CommandPerm)
	REGISTER(State::CoolDown)
	REGISTER(State::LastMessage)
	REGISTER(State::TriggerStatus)

	#undef REGISTER
	
	return v;
}


Group::Group(Mirai::GID_t group_id, Mirai::QQ_t owner_id,
	const std::vector<std::pair<std::string, int>>& command_list,
	const std::vector<std::pair<std::string, bool>>& trigger_list) 
	: gid(group_id), suid(owner_id), States(RegisterStates())
{
	auto command = this->GetState<State::CommandPerm>();
	for (const auto& p : command_list)
		command->AddCommand(p.first, p.second);

	auto trigger = this->GetState<State::TriggerStatus>();
	for (const auto& p : trigger_list)
		trigger->AddTrigger(p.first, p.second);

	this->FromFile();
	
	Utils::Timer::GetInstance().LaunchLoop([this]{ this->ToFile(); }, std::chrono::hours(1), true);
}

void Group::ToFile(void)
{
	json content;
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		for (const auto& p : this->States)
			if (!p.second->Serialize().empty())
				content["States"][p.first] = p.second->Serialize();
	}

	std::filesystem::path Path = "./bot";
	try
	{
		std::filesystem::create_directory(Path);
	}
	catch(const std::exception& e)
	{
		logging::WARN("Failed to create directory ./bot/: " + std::string(e.what()));
		return;
	}

	Path /= this->gid.to_string();
	{
		std::lock_guard<std::mutex> lk(this->mtx_file);
		std::ofstream file(Path);
		if (!file)
		{
			logging::WARN("Failed to open file " + string(Path) + " for writing");
			return;
		}
		logging::INFO("Writing to file " + string(Path));
		file << content.dump(1, '\t');
	}
}

void Group::FromFile(void)
{
	json content;
	std::filesystem::path Path = "./bot";
	Path /= this->gid.to_string();
	if (!std::filesystem::exists(Path))
		return;
	try
	{
		std::lock_guard<std::mutex> lk(this->mtx_file);
		std::ifstream file(Path);
		if (!file)
		{
			logging::WARN("Failed to open file " + string(Path) + " for reading");
			return;
		}
		content = json::parse(file);
	}
	catch(json::parse_error& e)
	{
		logging::WARN("Failed to parse file " + string(Path) + " :" + e.what());
		return;
	}
	logging::INFO("Reading from file " + string(Path));
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (content.contains("States"))
		{
			assert(content["States"].type() == json::value_t::object);
			for (const auto& p : content["States"].items())
				if (this->States.count(p.key()))
					this->States.at(p.key())->Deserialize(p.value());
		}
	}
}

}