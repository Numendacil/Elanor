add_plugin(SekaiPlugin)

target_sources(
	SekaiPlugin PRIVATE

	PluginMain.cpp
)

target_include_directories(SekaiPlugin PRIVATE .)

target_link_libraries(SekaiPlugin PRIVATE httplib::httplib)
target_link_libraries(SekaiPlugin PRIVATE libvips::libvips)
target_link_libraries(SekaiPlugin PRIVATE croncpp::croncpp)
target_link_libraries(SekaiPlugin PRIVATE OpenSSL::Crypto)
target_link_libraries(SekaiPlugin PRIVATE stduuid)

add_subdirectory(SekaiUpdater)
add_subdirectory(SekaiClient)
add_subdirectory(Trigger)

target_link_libraries(SekaiPlugin PRIVATE SekaiClient)

# add_subdirectory(Handlers)
# add_subdirectory(GroupCommand)
# add_subdirectory(PixivClient)
# add_subdirectory(Utils)