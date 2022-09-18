#ifndef _ELANOR_CORE_GROUP_HPP_
#define _ELANOR_CORE_GROUP_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <utility>
#include <vector>
#include <cassert>

#include <Core/States/StateBase.hpp>
#include <Core/Utils/Logger.hpp>
#include <libmirai/Types/BasicTypes.hpp>

namespace Bot
{

class Group
{
protected:
	mutable std::mutex _mtx_state;
	mutable std::mutex _mtx_file;

	const std::unordered_map<std::string, std::unique_ptr<State::StateBase>> _states;

	template<typename StateType> class _has_name_
	{
		using yes_type = char;
		using no_type = long;
		template<typename T> static yes_type test(decltype(&T::_NAME_));
		template<typename T> static no_type test(...);

	public:
		static constexpr bool value = sizeof(test<StateType>(0)) == sizeof(yes_type);
	};

public:
	Group(Mirai::GID_t group_id);
	Group(const Group&) = delete;
	Group& operator=(const Group&) = delete;
	Group(Group&&) = delete;
	Group& operator=(Group&&) = delete;

	void ToFile(const std::filesystem::path& filepath) const;
	void FromFile(const std::filesystem::path& filepath);

	const Mirai::GID_t gid;

	template<class T> T* GetState() const
	{
		static_assert(std::is_base_of<State::StateBase, T>::value, "T must be a derived class of StateBase.");
		static_assert(_has_name_<T>::value, "T must contain a static atrribute _NAME_");
		std::lock_guard<std::mutex> lk(this->_mtx_state);
		assert(this->_states.count(std::string(T::_NAME_)));
		assert(this->_states.at(std::string(T::_NAME_)));
		auto ptr = this->_states.at(std::string(T::_NAME_)).get();
		assert(ptr != nullptr);
		return static_cast<T*>(ptr);
	}

	~Group() = default;
};
} // namespace Bot

#endif