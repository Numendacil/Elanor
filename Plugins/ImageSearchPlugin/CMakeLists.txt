add_plugin(ImageSearch)

target_sources(
	ImageSearch PRIVATE

	PluginMain.cpp
)

target_include_directories(ImageSearch PRIVATE .)

target_link_libraries(ImageSearch PRIVATE libvips::libvips)
target_link_libraries(ImageSearch PRIVATE httplib::httplib)

add_subdirectory(SauceNAO)