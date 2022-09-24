#include "MorningTrigger.hpp"
#include "RollDice.hpp"

#define PLUGIN_ENTRY_IMPL
#include <Core/Interface/PluginEntry.hpp>

extern "C"
{

void InitPlugin()
{
}


const char* GetPluginName()
{
	return "CommonPlugin";
}

const char* GetPluginInfo()
{
	return "Plugin for basic functions";
}


int GetGroupCommandCount()
{
	return 1;
}

const char* GetGroupCommandName(int idx)
{
	switch(idx)
	{
	case 0:
		return GroupCommand::RollDice::_NAME_.data();
	default:
		return "";
	}
}

GroupCommand::IGroupCommand* GetGroupCommand(int idx)
{
	switch(idx)
	{
	case 0:
		return new GroupCommand::RollDice;
	default:
		return nullptr;
	}
}

void DeleteGroupCommand(GroupCommand::IGroupCommand* cmd)
{
	delete cmd;		// NOLINT(cppcoreguidelines-owning-memory)
}


int GetTriggerCount()
{
	return 1;
}

const char* GetTriggerName(int idx)
{
	switch(idx)
	{
	case 0:
		return Trigger::MorningTrigger::_NAME_.data();
	default:
		return "";
	}
}

Trigger::ITrigger* GetTrigger(int idx)
{
	switch(idx)
	{
	case 0:
		return new Trigger::MorningTrigger;
	default:
		return nullptr;
	}
}

void DeleteTrigger(Trigger::ITrigger* trigger)
{
	delete trigger;		// NOLINT(cppcoreguidelines-owning-memory)
}


void ClosePlugin()
{
}

}