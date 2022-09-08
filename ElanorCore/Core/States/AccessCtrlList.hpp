#ifndef _ELANOR_CORE_ACCESS_CONTROL_LIST_HPP_
#define _ELANOR_CORE_ACCESS_CONTROL_LIST_HPP_

#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>

#include <libmirai/Types/Types.hpp>

#include "StateBase.hpp"

namespace State
{

class AccessCtrlList : public StateBase
{
protected:
	mutable std::mutex _mtx;

	std::unordered_set<Mirai::QQ_t> _WhiteList;
	std::unordered_set<Mirai::QQ_t> _BlackList;

public:
	static constexpr std::string_view _NAME_ = "AccessCtrlList";

	bool IsWhiteList(const Mirai::QQ_t& id) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		return this->_WhiteList.contains(id);
	}

	void WhiteListAdd(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_WhiteList.insert(id);
	}

	void WhiteListDelete(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_WhiteList.erase(id);
	}

	void WhiteListClear()
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_WhiteList.clear();
	}

	std::vector<Mirai::QQ_t> GetWhiteList()
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		return {this->_WhiteList.begin(), this->_WhiteList.end()};
	}

	bool IsBlackList(const Mirai::QQ_t& id) const
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		return this->_BlackList.contains(id);
	}

	void BlackListAdd(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_BlackList.insert(id);
	}

	void BlackListDelete(const Mirai::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_BlackList.erase(id);
	}

	void BlackListClear()
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_BlackList.clear();
	}

	std::vector<Mirai::QQ_t> GetBlackList()
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		return {this->_BlackList.begin(), this->_BlackList.end()};
	}

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& content) override;
};

} // namespace State

#endif