add_executable(SekaiUpdater)

target_sources(
	SekaiUpdater

	PRIVATE
	main.cpp
	AcbDecoder.hpp
	AcbDecoder.cpp
	AssetDownloader.hpp
	AssetDownloader.cpp
	AssetUnpacker.hpp
	AssetUnpacker.cpp
	TaskDispatcher.hpp
	TaskDispatcher.cpp
)

add_subdirectory(cricpp)
add_subdirectory(pcm2mp3)

target_link_libraries(SekaiUpdater PRIVATE httplib::httplib)
target_link_libraries(SekaiUpdater PRIVATE cricpp)
target_link_libraries(SekaiUpdater PRIVATE pcm2mp3)

target_link_libraries(SekaiUpdater PRIVATE ElanorPlugins::PluginProperties)
set_target_properties(SekaiUpdater PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")

install(
	TARGETS SekaiUpdater
	DESTINATION "bin"
)