#ifndef _ELANOR_CORE_GROUP_LIST_HPP_
#define _ELANOR_CORE_GROUP_LIST_HPP_

#include <map>
#include <mutex>
#include <vector>

#include <libmirai/Types/BasicTypes.hpp>

#include "Group.hpp"

namespace Bot
{

class GroupList
{
protected:
	std::map<Mirai::GID_t, Group> _list;
	const std::vector<std::pair<std::string, int>> _command;
	const std::vector<std::pair<std::string, bool>> _trigger;
	Mirai::QQ_t _owner;

	mutable std::mutex _mtx;

public:
	GroupList(Mirai::QQ_t owner_id, std::vector<std::pair<std::string, int>> command_list,
	          std::vector<std::pair<std::string, bool>> trigger_list);

	Group& GetGroup(Mirai::GID_t gid);
	std::vector<Group*> GetAllGroups();
};

} // namespace Bot

#endif