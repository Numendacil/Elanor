#ifndef _BILILIVE_LIST_HPP_
#define _BILILIVE_LIST_HPP_

#include <map>
#include <vector>
#include <mutex>

#include <State/StateBase.hpp>

namespace State
{

class BililiveList : public StateBase
{
public:
	struct info
	{
		long room_id;
		bool broadcasted = false;
	};
protected:
	std::map<long, info> user_list;		// Map uid -> room_id
	mutable std::mutex mtx;
public:

	static constexpr std::string_view _NAME_ = "BililiveList";

	void Insert(long uid, long room_id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.emplace(uid, info{room_id, false});
	}

	void Erase(long uid)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.erase(uid);
	}

	void Broadcast(long uid)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.at(uid).broadcasted = true;
	}

	void ResetBroadcast(long uid)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.at(uid).broadcasted = false;
	}

	bool Exist(long uid) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->user_list.count(uid);
	}

	const info& GetInfo(long uid) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->user_list.at(uid);
	}

	std::vector<std::pair<long, info>> GetList(void) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return std::vector<std::pair<long, info>>(this->user_list.begin(), this->user_list.end());
	}

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;
};


}

#endif