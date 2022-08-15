#ifndef _GROUP_LIST_HPP_
#define _GROUP_LIST_HPP_

#include <map>
#include <mutex>
#include <vector>
#include <libmirai/Types/BasicTypes.hpp>
#include <Group/Group.hpp>

namespace Bot
{

class GroupList
{
protected:
	std::map<Mirai::GID_t, Group> group_list;
	const std::vector<std::pair<std::string, int>> command_list;
	const std::vector<std::pair<std::string, bool>> trigger_list;
	Mirai::QQ_t owner;

	mutable std::mutex mtx;
	
public:
	GroupList(Mirai::QQ_t owner_id, 
		const std::vector<std::pair<std::string, int>>& command_list,
		const std::vector<std::pair<std::string, bool>>& trigger_list);

	Group& GetGroup(Mirai::GID_t gid);
	std::vector<Group*> GetAllGroups();

};

}

#endif