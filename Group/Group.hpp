#ifndef _GROUP_HPP_
#define _GROUP_HPP_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <mutex>
#include <utility>
#include <vector>

#include <ThirdParty/log.h>
#include <State/StateBase.hpp>

#include <libmirai/Types/BasicTypes.hpp>

namespace Bot
{

template <typename T> 
class _has_name_
{
	typedef char yes_type;
	typedef long no_type;
	template <typename U> static yes_type test(decltype(&U::_NAME_));
	template <typename U> static no_type test(...);

	public:
	static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type);
};

class Group {

protected:
	mutable std::mutex mtx;
	mutable std::mutex mtx_file;

	const std::unordered_map<std::string, std::unique_ptr<State::StateBase>> States;
public:
	Group(Mirai::GID_t group_id, Mirai::QQ_t owner_id, 
		const std::vector<std::pair<std::string, int>>& command_list,
		const std::vector<std::pair<std::string, bool>>& trigger_list);
	void ToFile();
	void FromFile();

	const Mirai::GID_t gid;
	const Mirai::QQ_t suid;

	template<class T>
	T* GetState() const
	{
		static_assert(std::is_base_of<State::StateBase, T>::value, "T must be a derived class of StateBase.");
		static_assert(_has_name_<T>::value, "T must contain a static atrribute _NAME_");
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->States.count(std::string(T::_NAME_)));
		assert(this->States.at(std::string(T::_NAME_)));
		auto ptr = this->States.at(std::string(T::_NAME_)).get();
		assert(ptr != nullptr);
		return static_cast<T*>(ptr);
	}

	~Group() 
	{
		this->ToFile(); 
	}
};
}

#endif