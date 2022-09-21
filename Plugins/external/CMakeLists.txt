add_library(PluginExternal INTERFACE)
add_library(ElanorPlugins::External ALIAS PluginExternal)

add_subdirectory(croncpp)
add_subdirectory(utfcpp)

set(UUID_USING_CXX20_SPAN ON CACHE BOOL "Using span from std instead of gsl" FORCE)
add_subdirectory(stduuid)

target_link_libraries(PluginExternal INTERFACE croncpp::croncpp)
target_link_libraries(PluginExternal INTERFACE utf8::cpp)
target_link_libraries(PluginExternal INTERFACE stduuid)

target_link_libraries(PluginExternal INTERFACE httplib)