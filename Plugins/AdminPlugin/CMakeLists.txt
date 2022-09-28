add_plugin(AdminPlugin)

target_sources(
	AdminPlugin PRIVATE

	PluginMain.cpp
)

target_include_directories(AdminPlugin PRIVATE .)

add_subdirectory(GroupCommand)