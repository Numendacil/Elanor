add_plugin(CommonPlugin)

target_sources(
	CommonPlugin PRIVATE

	PluginMain.cpp
)

add_subdirectory(GroupCommand)
add_subdirectory(Trigger)