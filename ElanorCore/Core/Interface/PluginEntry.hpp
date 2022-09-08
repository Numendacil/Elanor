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
	EXPORTED void InitPlugin();

	EXPORTED const char* GetPluginName();
	EXPORTED const char* GetPluginInfo();

	EXPORTED int GetGroupCommandCount();
	EXPORTED const char* GetGroupCommandName(int idx);
	EXPORTED GroupCommand::IGroupCommand* GetGroupCommand(int idx);
	EXPORTED void DeleteGroupCommand(GroupCommand::IGroupCommand* cmd);

	EXPORTED int GetTriggerCount();
	EXPORTED const char* GetTriggerName(int idx);
	EXPORTED Trigger::ITrigger* GetTrigger(int idx);
	EXPORTED void DeleteTrigger(Trigger::ITrigger* trigger);

	EXPORTED void ClosePlugin();
}

#endif