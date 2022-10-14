#ifndef _ELANOR_CORE_PLUGIN_ENTRY_HPP_
#define _ELANOR_CORE_PLUGIN_ENTRY_HPP_

#include "IGroupCommand.hpp"
#include "ITrigger.hpp"

#if defined(__linux__) || defined(__APPLE__)
#define EXPORTED __attribute__((visibility("default")))
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

		size_t (*GetGroupCommandCount)();
		const char* (*GetGroupCommandName)(size_t);
		GroupCommand::IGroupCommand* (*GetGroupCommand)(size_t);
		void (*DeleteGroupCommand)(GroupCommand::IGroupCommand*);

		size_t (*GetTriggerCount)();
		const char* (*GetTriggerName)(size_t);
		Trigger::ITrigger* (*GetTrigger)(size_t);
		void (*DeleteTrigger)(Trigger::ITrigger*);

		void (*ClosePlugin)();
	};

#ifdef PLUGIN_ENTRY_IMPL

	void InitPlugin();

	const char* GetPluginName();
	const char* GetPluginInfo();

	size_t GetGroupCommandCount();
	const char* GetGroupCommandName(size_t idx);
	GroupCommand::IGroupCommand* GetGroupCommand(size_t idx);
	void DeleteGroupCommand(GroupCommand::IGroupCommand* cmd);

	size_t GetTriggerCount();
	const char* GetTriggerName(size_t idx);
	Trigger::ITrigger* GetTrigger(size_t idx);
	void DeleteTrigger(Trigger::ITrigger* trigger);

	void ClosePlugin();

	extern "C" EXPORTED const API ApiTable{
		InitPlugin,         GetPluginName,   GetPluginInfo,  GetGroupCommandCount, GetGroupCommandName, GetGroupCommand,
		DeleteGroupCommand, GetTriggerCount, GetTriggerName, GetTrigger,           DeleteTrigger,       ClosePlugin};

#endif
}

#endif