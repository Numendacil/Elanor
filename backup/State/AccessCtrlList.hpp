#ifndef _ACCESS_CONTROL_LIST_HPP_
#define _ACCESS_CONTROL_LIST_HPP_

#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>

#include <libmirai/Types/Types.hpp>
#include <State/StateBase.hpp>

namespace State
{

class AccessCtrlList : public StateBase
{
protected:
	mutable std::mutex mtx;
	
	std::unordered_set<Mirai::QQ_t> WhiteList;
	std::unordered_set<Mirai::QQ_t> BlackList;

public:

	static constexpr std::string_view _NAME_ = "AccessCtrlList";

	bool IsWhiteList(const Mirai::QQ_t& id) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->WhiteList.contains(id);
	}

	void WhiteListAdd(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->WhiteList.insert(id);
	}

	void WhiteListDelete(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->WhiteList.erase(id);
	}

	void WhiteListClear(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->WhiteList.clear();
	}

	std::vector<Mirai::QQ_t> GetWhiteList(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return std::vector<Mirai::QQ_t>(this->WhiteList.begin(), this->WhiteList.end());
	}

	bool IsBlackList(const Mirai::QQ_t& id) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->BlackList.contains(id);
	}

	void BlackListAdd(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->BlackList.insert(id);
	}

	void BlackListDelete(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->BlackList.erase(id);
	}

	void BlackListClear(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->BlackList.clear();
	}
	
	std::vector<Mirai::QQ_t> GetBlackList(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return std::vector<Mirai::QQ_t>(this->BlackList.begin(), this->BlackList.end());
	}

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;

};

}

#endif