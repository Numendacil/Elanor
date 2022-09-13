#ifndef _ELANOR_CORE_PLUGIN_ENTRY_HPP_
#define _ELANOR_CORE_PLUGIN_ENTRY_HPP_

#include "IGroupCommand.hpp"
#include "ITrigger.hpp"

#if defined(__linux__) || defined(__APPLE__)
#define EXPORTED
#endif

#if defined(_WIN32)
	#if defined(MIRAI_BUILD_DLL)
		#define EXPORTED __declspec(dllexport)
	#else
		#define EXPORTED
	#endif
#endif

#ifndef EXPORTED
#error Unsupported platform
#endif

extern "C"
{
	struct API
	{
		void (*InitPlugin)();

		const char* (*GetPluginName)();
		const char* (*GetPluginInfo)();

		int (*GetGroupCommandCount)();
		const char* (*GetGroupCommandName)(int);
		GroupCommand::IGroupCommand* (*GetGroupCommand)(int);
		void (*DeleteGroupCommand)(GroupCommand::IGroupCommand*);

		int (*GetTriggerCount)();
		const char* (*GetTriggerName)(int);
		Trigger::ITrigger* (*GetTrigger)(int);
		void (*DeleteTrigger)(Trigger::ITrigger*);

		void (*ClosePlugin)();
	};

#ifdef PLUGIN_ENTRY_IMPL

	void InitPlugin();

	const char* GetPluginName();
	const char* GetPluginInfo();

	int GetGroupCommandCount();
	const char* GetGroupCommandName(int idx);
	GroupCommand::IGroupCommand* GetGroupCommand(int idx);
	void DeleteGroupCommand(GroupCommand::IGroupCommand* cmd);

	int GetTriggerCount();
	const char* GetTriggerName(int idx);
	Trigger::ITrigger* GetTrigger(int idx);
	void DeleteTrigger(Trigger::ITrigger* trigger);

	void ClosePlugin();

	extern "C" EXPORTED const API ApiTable{
		InitPlugin, 
		GetPluginName,
		GetPluginInfo,
		GetGroupCommandCount,
		GetGroupCommandName,
		GetGroupCommand,
		DeleteGroupCommand,
		GetTriggerCount,
		GetTriggerName,
		GetTrigger,
		DeleteTrigger,
		ClosePlugin
	};

#endif

}

#endif