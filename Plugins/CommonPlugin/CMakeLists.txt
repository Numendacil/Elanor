add_plugin(CommonPlugin)

target_sources(
	CommonPlugin PRIVATE

	PluginMain.cpp
)


target_include_directories(CommonPlugin PRIVATE .)

add_subdirectory(GroupCommand)
add_subdirectory(Trigger)