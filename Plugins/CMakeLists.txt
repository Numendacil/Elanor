cmake_minimum_required (VERSION 3.20)

project (ElanorPlugins VERSION 1.0.0 LANGUAGES CXX)

add_library(PluginProperties INTERFACE)
add_library(ElanorPlugins::PluginProperties ALIAS PluginProperties)

add_subdirectory(external)

# Check corresponding targets
if(NOT TARGET croncpp::croncpp)
	message(WARNING "TARGET croncpp::croncpp not found")
endif()

if(NOT TARGET utf8::cpp)
	message(WARNING "TARGET utf8::cpp not found")
endif()

if(NOT TARGET stduuid)
	message(WARNING "TARGET stduuid not found")
endif()

if(NOT TARGET httplib::httplib)
	message(WARNING "TARGET httplib::httplib not found")
endif()

if(NOT TARGET libvips::libvips)
	message(WARNING "TARGET libvips::libvips not found")
endif()

if(NOT TARGET ZLIB::ZLIB)
	message(WARNING "TARGET ZLIB::ZLIB not found")
endif()

if(NOT TARGET OpenSSL::SSL)
	message(WARNING "TARGET OpenSSL::SSL not found")
endif()

if(NOT TARGET OpenSSL::Crypto)
	message(WARNING "TARGET OpenSSL::Crypto not found")
endif()

target_link_libraries(PluginProperties INTERFACE ${CMAKE_PROJECT_NAME}::ElanorCore)

function(add_plugin plugin_name)
	add_library(${plugin_name} SHARED)

	target_link_libraries(${plugin_name} PRIVATE ElanorPlugins::PluginProperties)
	set_target_properties(${plugin_name} PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")
	set_target_properties(${plugin_name} PROPERTIES PREFIX "")
	set_target_properties(${plugin_name} PROPERTIES CXX_VISIBILITY_PRESET hidden)

	install(
		TARGETS ${plugin_name}
		DESTINATION "lib"
	)
	install(
		TARGETS ${plugin_name}
		DESTINATION "bin/Plugins"
	)
endfunction()

add_subdirectory(Utils)
add_subdirectory(AdminPlugin)
add_subdirectory(BilibiliPlugin)
add_subdirectory(CommonPlugin)
add_subdirectory(ImageSearchPlugin)
add_subdirectory(MemePlugin)
add_subdirectory(PixivPlugin)

install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pymodules"
	DESTINATION "bin"
	PATTERN "__pycache__" EXCLUDE
)