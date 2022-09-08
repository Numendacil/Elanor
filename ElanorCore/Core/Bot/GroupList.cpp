#include "GroupList.hpp"

#include <cassert>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <Core/Utils/Logger.hpp>

using std::pair;
using std::string;
using std::vector;

namespace Bot
{

GroupList::GroupList(Mirai::QQ_t owner_id, std::vector<std::pair<std::string, int>> command_list,
                     std::vector<std::pair<std::string, bool>> trigger_list)
	: _command(std::move(command_list)), _trigger(std::move(trigger_list)), _owner(owner_id)
{
	string path = "./bot";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			try
			{
				Mirai::GID_t gid = (Mirai::GID_t)std::stol(entry.path().stem());
				this->_list.try_emplace(gid, gid, this->_owner, this->_command, this->_trigger);
			}
			catch (const std::logic_error& e)
			{
				LOG_WARN(Utils::GetLogger(),
				         "Unexpected file found in ./bot directory: " + string(entry.path().filename()));
			}
		}
	}
}

Group& GroupList::GetGroup(Mirai::GID_t gid)
{
	std::lock_guard<std::mutex> lk(this->_mtx);
	auto it = this->_list.find(gid);
	if (it != this->_list.end()) return it->second;
	else
	{
		auto result = this->_list.try_emplace(gid, gid, this->_owner, this->_command, this->_trigger);
		assert(result.second);
		return result.first->second;
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