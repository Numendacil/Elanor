target_sources(
	PluginProperties INTERFACE 
	
	PluginUtils/Base64.hpp
	PluginUtils/Common.hpp
	PluginUtils/NetworkUtils.hpp
	PluginUtils/StringUtils.hpp
	PluginUtils/TypeList.hpp
	PluginUtils/UrlComponents.hpp
)

target_include_directories(PluginProperties INTERFACE .)