#include <Utils/TypeList.hpp>

#include "GroupCommand/Answer.hpp"
#include "GroupCommand/AtBot.hpp"
#include "GroupCommand/Recall.hpp"
#include "GroupCommand/Repeat.hpp"
#include "GroupCommand/RollDice.hpp"
#include "Trigger/MorningTrigger.hpp"

#define PLUGIN_ENTRY_IMPL
#include <Core/Interface/PluginEntry.hpp>

using GroupCommandList = Utils::TypeList< 
	GroupCommand::Answer,
	GroupCommand::AtBot,
	GroupCommand::Recall,
	GroupCommand::Repeat,
	GroupCommand::RollDice
>;
using TriggerList = Utils::TypeList< Trigger::MorningTrigger >;

extern "C"
{

	void InitPlugin() {}


	const char* GetPluginName()
	{
		return "CommonPlugin";
	}

	const char* GetPluginInfo()
	{
		return "Plugin for basic functions";
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
		return 1;
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