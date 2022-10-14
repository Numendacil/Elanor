add_plugin(BilibiliPlugin)

target_sources(
	BilibiliPlugin PRIVATE

	PluginMain.cpp
)


target_include_directories(BilibiliPlugin PRIVATE .)

target_link_libraries(BilibiliPlugin PRIVATE httplib::httplib)

add_subdirectory(GroupCommand)
add_subdirectory(State)
add_subdirectory(Trigger)