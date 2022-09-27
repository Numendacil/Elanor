cmake_minimum_required (VERSION 3.20)

project (ElanorPlugins VERSION 1.0.0 LANGUAGES CXX)

add_library(PluginProperties INTERFACE)
add_library(ElanorPlugins::PluginProperties ALIAS PluginProperties)

add_subdirectory(external)

target_link_libraries(PluginProperties INTERFACE ElanorPlugins::External)
target_link_libraries(PluginProperties INTERFACE ${CMAKE_PROJECT_NAME}::ElanorCore)
target_include_directories(PluginProperties INTERFACE .)

function(add_plugin plugin_name)
	add_library(${plugin_name} SHARED)

	target_link_libraries(${plugin_name} PRIVATE ElanorPlugins::PluginProperties)
	set_target_properties(${plugin_name} PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")
	set_target_properties(${plugin_name} PROPERTIES PREFIX "")

	install(
		TARGETS ${plugin_name}
		DESTINATION "lib"
	)
	install(
		TARGETS ${plugin_name}
		DESTINATION "bin/Plugins"
	)
endfunction()

add_subdirectory(Utils)
add_subdirectory(AdminPlugin)
add_subdirectory(CommonPlugin)