#include "GroupList.hpp"

#include <cassert>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <Core/Utils/Logger.hpp>
#include <Core/States/CommandPerm.hpp>
#include <Core/States/TriggerStatus.hpp>
#include <Core/States/AccessCtrlList.hpp>

using std::pair;
using std::string;
using std::vector;

namespace Bot
{

void GroupList::LoadGroups(std::filesystem::path folder)
{
	for (const auto& entry : std::filesystem::directory_iterator(folder))
	{
		if (entry.is_regular_file())
		{
			try
			{
				Mirai::GID_t gid = (Mirai::GID_t)std::stol(entry.path().stem());

				std::lock_guard<std::mutex> lk(this->_mtx);
				auto it = this->_list.find(gid);
				if (it != this->_list.end()) 
					it->second.FromFile(entry.path());
				else
				{
					auto result = this->_list.try_emplace(gid, gid);
					assert(result.second);
					Group& group = result.first->second;

					auto command = group.GetState<State::CommandPerm>();
					for (const auto& p : this->_command)
					{
						command->AddCommand(p.first, p.second);
					}

					auto trigger = group.GetState<State::TriggerStatus>();
					for (const auto& p : this->_trigger)
					{
						trigger->AddTrigger(p.first, p.second);
					}

					auto ACList = group.GetState<State::AccessCtrlList>();
					ACList->SetSuid(this->_suid);

					group.FromFile(entry.path());
				}
			}
			catch (const std::logic_error& e)
			{
				LOG_WARN(Utils::GetLogger(),
				         "Unexpected file found in " + string(folder) + " directory: " + string(entry.path().filename()));
			}
		}
	}
}

void GroupList::SetCommands(vector<pair<string, int>> command_list)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	this->_command = std::move(command_list);

	for (auto&& [gid, group] : this->_list)
	{
		auto command = group.GetState<State::CommandPerm>();
		for (const auto& p : this->_command)
		{
			if (command->ExistCommand(p.first))
			{
				int perm = command->GetPermission(p.first).first;
				command->AddCommand(p.first, p.second);
				command->UpdatePermission(p.first, perm);
			}
			else
				command->AddCommand(p.first, p.second);
		}
	}
}

void GroupList::SetTriggers(vector<pair<string, bool>> trigger_list)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	this->_trigger = std::move(trigger_list);

	for (auto&& [gid, group] : this->_list)
	{
		auto trigger = group.GetState<State::TriggerStatus>();
		for (const auto& p : this->_trigger)
		{
			if (!trigger->ExistTrigger(p.first))
			{
				trigger->AddTrigger(p.first, p.second);
			}
		}
	}
}

void GroupList::SetSuid(Mirai::QQ_t id)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	this->_suid = id;

	for (auto&& [gid, group] : this->_list)
	{
		auto ACList = group.GetState<State::AccessCtrlList>();
		ACList->SetSuid(this->_suid);
	}
}

Group& GroupList::GetGroup(Mirai::GID_t gid)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	auto it = this->_list.find(gid);
	if (it != this->_list.end()) return it->second;
	else
	{
		auto result = this->_list.try_emplace(gid, gid);
		assert(result.second);
		Group& group = result.first->second;

		auto command = group.GetState<State::CommandPerm>();
		for (const auto& p : this->_command)
		{
			command->AddCommand(p.first, p.second);
		}

		auto trigger = group.GetState<State::TriggerStatus>();
		for (const auto& p : this->_trigger)
		{
			trigger->AddTrigger(p.first, p.second);
		}

		auto ACList = group.GetState<State::AccessCtrlList>();
		ACList->SetSuid(this->_suid);

		return group;
	}
}

std::vector<Group*> GroupList::GetAllGroups()
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	std::vector<Group*> v;
	v.reserve(this->_list.size());
	for (auto& p : this->_list)
		v.push_back(&p.second);
	return v;
}

} // namespace Bot