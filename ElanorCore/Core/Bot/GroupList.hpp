#ifndef _ELANOR_CORE_GROUP_LIST_HPP_
#define _ELANOR_CORE_GROUP_LIST_HPP_

#include <map>
#include <mutex>
#include <utility>
#include <vector>

#include <libmirai/Types/BasicTypes.hpp>

#include "Group.hpp"

namespace Bot
{

class GroupList
{
protected:
	std::map<Mirai::GID_t, Group> _list;
	std::vector<std::pair<std::string, int>> _command;
	std::vector<std::pair<std::string, bool>> _trigger;
	Mirai::QQ_t _suid{};

	mutable std::mutex _mtx;

public:
	GroupList() = default;
	GroupList(const GroupList&) = delete;
	GroupList& operator=(const GroupList&) = delete;
	GroupList(GroupList&&) = delete;
	GroupList& operator=(GroupList&&) = delete;

	void LoadGroups(std::filesystem::path folder);
	void SetCommands(std::vector<std::pair<std::string, int>> command_list);
	void SetTriggers(std::vector<std::pair<std::string, bool>> trigger_list);
	void SetSuid(Mirai::QQ_t id);

	Group& GetGroup(Mirai::GID_t gid);
	std::vector<Group*> GetAllGroups();

	~GroupList() = default;
};

} // namespace Bot

#endif