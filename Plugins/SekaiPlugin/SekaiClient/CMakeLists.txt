add_library(SekaiClient STATIC)

target_sources(
	SekaiClient

	PRIVATE
	SekaiClient.hpp
	SekaiClient.cpp
	Singleton.hpp
	Singleton.cpp
)

add_subdirectory(SekaiNetworkClient)
add_subdirectory(models)

target_include_directories(SekaiClient PRIVATE .)

target_link_libraries(SekaiClient PRIVATE httplib::httplib)
target_link_libraries(SekaiClient PRIVATE OpenSSL::Crypto)
target_link_libraries(SekaiClient PRIVATE stduuid)
target_link_libraries(SekaiClient PRIVATE ElanorPlugins::PluginProperties)