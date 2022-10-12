add_plugin(CommonPlugin)

target_sources(
	CommonPlugin PRIVATE

	PluginMain.cpp
)


target_include_directories(CommonPlugin PRIVATE .)

target_link_libraries(CommonPlugin PRIVATE croncpp::croncpp)

add_subdirectory(GroupCommand)
add_subdirectory(Trigger)