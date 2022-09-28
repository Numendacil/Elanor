#ifndef _UTILS_TYPELIST_HPP_
#define _UTILS_TYPELIST_HPP_

#include <cstddef>

namespace Utils
{

template<typename... Types> struct TypeList;

template<size_t Idx, typename... Types> class _GetTypeAt_
{

	template<size_t _Idx, typename... _Types> struct _GetTypeAtImpl_;

	template<typename _Type, typename... _Types> struct _GetTypeAtImpl_<0, _Type, _Types...>
	{
		using type = _Type;
	};

	template<size_t _Idx, typename _Type, typename... _Types>
	struct _GetTypeAtImpl_<_Idx, _Type, _Types...> : public _GetTypeAtImpl_<_Idx - 1, _Types...>
	{
	};

public:
	static_assert(Idx < sizeof...(Types), "Index out of bounds");
	using type = typename _GetTypeAtImpl_<Idx, Types...>::type;
};

template<typename... Types> struct TypeList
{
	template<size_t Idx> using At = _GetTypeAt_<Idx, Types...>;

	template<size_t Idx> using At_t = typename _GetTypeAt_<Idx, Types...>::type;

	static constexpr size_t size = sizeof...(Types);
};


} // namespace Utils


#endif