cmake_minimum_required (VERSION 3.20)

project (ElanorPlugins VERSION 1.0.0 LANGUAGES CXX)

add_library(PluginProperties INTERFACE)
add_library(ElanorPlugins::PluginProperties ALIAS PluginProperties)

add_subdirectory(external)

target_link_libraries(PluginProperties INTERFACE ElanorPlugins::External)
target_link_libraries(PluginProperties INTERFACE ${CMAKE_PROJECT_NAME}::ElanorCore)

add_library(Plugins INTERFACE)
set_target_properties(Plugins PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")

function(add_plugin plugin_name)
	add_library(${plugin_name} SHARED)

	set_target_properties(${plugin_name} PROPERTIES PREFIX "")
	target_link_libraries(${plugin_name} PRIVATE ElanorPlugins::PluginProperties)
	target_link_libraries(Plugins INTERFACE ${plugin_name})
	target_include_directories(${plugin_name} PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
endfunction()

add_subdirectory(Utils)
add_subdirectory(CommonPlugin)

install(
	TARGETS Plugins
	DESTINATION "lib"
)
install(
	TARGETS Plugins
	DESTINATION "bin/Plugins"
)