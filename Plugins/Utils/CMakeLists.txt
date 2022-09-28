target_sources(
	PluginProperties INTERFACE 
	
	PluginUtils/Common.hpp
	PluginUtils/Common.cpp
	PluginUtils/StringUtils.hpp
	PluginUtils/StringUtils.cpp

	PluginUtils/TypeList.hpp
)

target_include_directories(PluginProperties INTERFACE .)