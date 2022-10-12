add_plugin(PixivPlugin)

target_sources(
	PixivPlugin PRIVATE

	PluginMain.cpp
)

target_include_directories(PixivPlugin PRIVATE .)

target_link_libraries(PixivPlugin PRIVATE httplib::httplib)
target_link_libraries(PixivPlugin PRIVATE libvips::libvips)
target_link_libraries(PixivPlugin PRIVATE OpenSSL::Crypto)
target_link_libraries(PixivPlugin PRIVATE stduuid)

add_subdirectory(Functions)
add_subdirectory(GroupCommand)
add_subdirectory(PixivClient)