target_include_directories(PluginProperties INTERFACE .)

add_subdirectory(croncpp)
add_subdirectory(utfcpp)

set(UUID_USING_CXX20_SPAN ON CACHE BOOL "Using span from std instead of gsl" FORCE)
add_subdirectory(stduuid)



if(NOT TARGET httplib::httplib)
find_package(httplib REQUIRED)
# else build httplib from local subdirectory
set_target_properties(httplib::httplib PROPERTIES IMPORTED_GLOBAL TRUE)
endif()

find_package(ZLIB REQUIRED)
set_target_properties(ZLIB::ZLIB PROPERTIES IMPORTED_GLOBAL TRUE)
find_package(OpenSSL REQUIRED)
set_target_properties(OpenSSL::SSL PROPERTIES IMPORTED_GLOBAL TRUE)
set_target_properties(OpenSSL::Crypto PROPERTIES IMPORTED_GLOBAL TRUE)


add_library(libvips INTERFACE)
add_library(libvips::libvips ALIAS libvips)
find_package(PkgConfig REQUIRED)

pkg_check_modules(VIPS REQUIRED vips-cpp)

target_include_directories(libvips INTERFACE ${VIPS_INCLUDE_DIRS})
target_link_directories(libvips INTERFACE ${VIPS_LIBRARY_DIRS})
target_link_libraries(libvips INTERFACE ${VIPS_LIBRARIES})