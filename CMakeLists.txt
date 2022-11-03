cmake_minimum_required (VERSION 3.20)

project(ElanorBot LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install library in the current directory" FORCE)

add_compile_definitions(MIRAI_LOGGING_LEVEL=1)
# add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
# add_link_options(-fsanitize=address -fno-omit-frame-pointer)

# UnitTests
option(ELANOR_BUILD_UNIT_TESTS "Build UnitTests" ON)
if(ELANOR_BUILD_UNIT_TESTS)
	message("UnitTests enabled in Elanor")
	add_subdirectory(googletest)

	add_library(GoogleTestLibs INTERFACE)
	target_link_libraries(GoogleTestLibs INTERFACE GTest::gtest_main GTest::gtest)
	target_compile_options(GoogleTestLibs INTERFACE -fsanitize=address -fno-omit-frame-pointer)
	target_link_options(GoogleTestLibs INTERFACE -fsanitize=address -fno-omit-frame-pointer)

	set(ELANOR_BUILD_MOCK_LIB ON CACHE BOOL "Build MockLibs" FORCE)

	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	enable_testing()
	include(GoogleTest)
endif(ELANOR_BUILD_UNIT_TESTS)

add_subdirectory(ElanorApp)
add_subdirectory(ElanorCore)
add_subdirectory(Plugins)