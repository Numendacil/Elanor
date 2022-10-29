#include <PluginUtils/TypeList.hpp>
#include <GroupCommand/Choyen.hpp>
#include <GroupCommand/Petpet.hpp>
#include <vips/vips8>

#define PLUGIN_ENTRY_IMPL
#include <Core/Interface/PluginEntry.hpp>

using GroupCommandList = Utils::TypeList< 
	GroupCommand::Choyen, 
	GroupCommand::Petpet 
>;
using TriggerList = Utils::TypeList<>;

extern "C"
{

	void InitPlugin() 
	{
		VIPS_INIT("");
	}


	const char* GetPluginName()
	{
		return "PixivPlugin";
	}

	const char* GetPluginInfo()
	{
		return "Plugin for Pixiv related functions";
	}


	size_t GetGroupCommandCount()
	{
		return GroupCommandList::size;
	}

	const char* GetGroupCommandName(size_t idx)
	{
		const char* name = "";
		[&]<size_t... Is>(std::integer_sequence<size_t, Is...> const&)
		{
			(void)((idx == Is ? (name = GroupCommandList::At<Is>::type::_NAME_.data(), true) : false) || ...);
		}
		(std::make_integer_sequence<size_t, GroupCommandList::size>{});

		return name;
	}

	GroupCommand::IGroupCommand* GetGroupCommand(size_t idx)
	{
		GroupCommand::IGroupCommand* command = nullptr;
		[&]<size_t... Is>(std::integer_sequence<size_t, Is...> const&)
		{
			(void)((idx == Is ? (command = new GroupCommandList::At_t<Is>, true) : false)|| ...); // NOLINT(cppcoreguidelines-owning-memory)
		}
		(std::make_integer_sequence<size_t, GroupCommandList::size>{});

		return command;
	}

	void DeleteGroupCommand(GroupCommand::IGroupCommand* cmd)
	{
		delete cmd; // NOLINT(cppcoreguidelines-owning-memory)
	}


	size_t GetTriggerCount()
	{
		return TriggerList::size;
	}

	const char* GetTriggerName(size_t idx)
	{
		const char* name = "";
		[&]<size_t... Is>(std::integer_sequence<size_t, Is...> const&)
		{
			(void)((idx == Is ? (name = TriggerList::At<Is>::type::_NAME_.data(), true) : false) || ...);
		}
		(std::make_integer_sequence<size_t, TriggerList::size>{});

		return name;
	}

	Trigger::ITrigger* GetTrigger(size_t idx)
	{
		Trigger::ITrigger* trigger = nullptr;
		[&]<size_t... Is>(std::integer_sequence<size_t, Is...> const&)
		{
			(void)((idx == Is ? (trigger = new TriggerList::At_t<Is>, true) : false) || ...); // NOLINT(cppcoreguidelines-owning-memory)
		}
		(std::make_integer_sequence<size_t, TriggerList::size>{});

		return trigger;
	}

	void DeleteTrigger(Trigger::ITrigger* trigger)
	{
		delete trigger; // NOLINT(cppcoreguidelines-owning-memory)
	}


	void ClosePlugin() {}
}