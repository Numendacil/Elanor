add_plugin(AdminPlugin)

target_sources(
	AdminPlugin PRIVATE

	PluginMain.cpp
)

add_subdirectory(GroupCommand)