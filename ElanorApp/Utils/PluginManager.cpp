#include "PluginManager.hpp"

#include <stdexcept>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{
[[nodiscard]] inline void* load_dynamic_library(const std::string& library_name)
{
#ifdef _WIN32
	return reinterpret_cast<void*>(LoadLibraryA(library_name.c_str()));
#else
	return dlopen(library_name.c_str(), RTLD_NOW);
#endif
}

inline void unload_dynamic_library(void* lib_handle)
{
#ifdef _WIN32
	FreeLibrary(HMODULE(lib_handle));
#else
	dlclose(lib_handle);
#endif
}

[[nodiscard]] inline void* get_sym_address(void* lib_handle, const std::string& function_name)
{

#ifdef _WIN32
	return reinterpret_cast<void*>(GetProcAddress(HMODULE(lib_handle), LPCSTR(function_name.c_str())));
#else
	return dlsym(lib_handle, function_name.c_str());
#endif
}
} // namespace

PluginLibrary& PluginLibrary::operator=(PluginLibrary&& rhs) noexcept
{
	if (&rhs == this) return *this;
	if (this->handle != nullptr) this->Close();
	this->handle = rhs.handle;
	rhs.handle = nullptr;
	return *this;
}

void PluginLibrary::Open(const std::string& libpath)
{
	if (this->handle != nullptr) this->Close();
	this->handle = load_dynamic_library(libpath);
	if (!this->handle) throw std::runtime_error("Failed to load library " + std::string(libpath));
}

void PluginLibrary::Close()
{
	if (!this->handle) return;

	unload_dynamic_library(this->handle);
	this->handle = nullptr;
}

void* PluginLibrary::GetSym(const char* name) const
{
	return get_sym_address(this->handle, name);
}