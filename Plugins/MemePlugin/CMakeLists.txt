add_plugin(MemePlugin)

target_sources(
	MemePlugin PRIVATE

	PluginMain.cpp
)

target_include_directories(MemePlugin PRIVATE .)

target_link_libraries(MemePlugin PRIVATE libvips::libvips)
target_link_libraries(MemePlugin PRIVATE httplib::httplib)

add_subdirectory(GroupCommand)