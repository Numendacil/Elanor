#ifndef _PLUGIN_MANAGER_
#define _PLUGIN_MANAGER_

#include <filesystem>
#include <string>
#include <type_traits>

class PluginLibrary
{
protected:
	void* handle = nullptr;


public:
	PluginLibrary() = default;
	PluginLibrary(const std::string& libpath) { this->Open(libpath); }
	PluginLibrary(const PluginLibrary&) = delete;
	PluginLibrary& operator=(const PluginLibrary&) = delete;

	PluginLibrary& operator=(PluginLibrary&& rhs) noexcept;

	PluginLibrary(PluginLibrary&& rhs) noexcept { *this = std::move(rhs); }

	~PluginLibrary() { this->Close(); }

	void Open(const std::string& libpath);
	void Close();

	[[nodiscard]] void* GetSym(const char* name) const;
};

#endif