#ifndef _UTILS_COMMON_HPP_
#define _UTILS_COMMON_HPP_

#include <optional>
#include <random>

#include <nlohmann/json.hpp>

#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/States/AccessCtrlList.hpp>
#include <Core/Utils/Logger.hpp>

namespace Bot
{

class Group;

}

namespace Utils
{

inline bool CheckAuth(const Mirai::GroupMember& member, const Bot::Group& group, int permission)
{
	auto ACList = group.GetState<State::AccessCtrlList>();

	if (ACList->GetSuid() == member.id) return true;
	if (ACList->IsBlackList(member.id)) return false;

	int auth = 0;
	switch (member.permission)
	{
	case Mirai::PERMISSION::OWNER:
		auth = 30; // NOLINT(*-avoid-magic-numbers)
		break;
	case Mirai::PERMISSION::ADMINISTRATOR:
		auth = 20; // NOLINT(*-avoid-magic-numbers)
		break;
	case Mirai::PERMISSION::MEMBER:
		auth = 10; // NOLINT(*-avoid-magic-numbers)
		break;
	default:
		auth = 0; // NOLINT(*-avoid-magic-numbers)
		break;
	}

	if (ACList->IsWhiteList(member.id)) auth = 50; // NOLINT(*-avoid-magic-numbers)

	return auth >= permission;
}

inline std::string GetDescription(const Mirai::GroupMember& member, bool from = true)
{
	std::string member_str = member.MemberName + "(" + member.id.to_string() + ")";
	std::string group_str = member.group.name + "(" + member.group.id.to_string() + ")";
	return ((from) ? "\t<- [" : "\t-> [") + member_str + ", " + group_str + "]";
}

inline std::string GetDescription(const Mirai::User& user, bool from = true)
{
	std::string profile_str = user.nickname + "(" + user.id.to_string() + ")";
	return ((from) ? "\t<- [" : "\t-> [") + profile_str + "]";
}

inline std::string GetText(const Mirai::MessageChain& msg)
{
	std::string text;
	for (const auto& p : msg)
	{
		if (p->GetType() == Mirai::PlainMessage::_TYPE_)
		{
			text += static_cast<Mirai::PlainMessage*>(p.get())->GetText(); // NOLINT(*-static-cast-downcast)
		}
	}
	return text;
}

template<typename ValueType, typename KeyType>
inline auto GetValue(const nlohmann::json& j, KeyType&& key, ValueType&& default_value)
{
	using ReturnType = decltype(std::declval<typename nlohmann::json>().value(std::forward<KeyType>(key),
	                                                                          std::forward<ValueType>(default_value)));
	if (!j.is_object() || !j.contains(key) || j.at(key).is_null())
		return (ReturnType)std::forward<ValueType>(default_value);
	return j.at(key).template get<ReturnType>();
}

template<typename ValueType, typename KeyType>
inline std::optional<ValueType> GetOptional(const nlohmann::json& j, KeyType&& key)
{
	if (!j.is_object() || !j.contains(key) || j.at(key).is_null())
	{
		return std::nullopt;
	}
	return j.at(key).template get<ValueType>();
}

template<typename KeyType> 
inline bool HasValue(const nlohmann::json& j, KeyType&& key)
{
	return !(!j.is_object() || !j.contains(key) || j.at(key).is_null());
}

} // namespace Utils

#endif
