cmake_minimum_required (VERSION 3.20)

project(ElanorBot LANGUAGES CXX VERSION 1.0.0)



# Options
option(ELANOR_BUILD_UNIT_TESTS "Build UnitTests" ON)
option(ELANOR_INSTALL "Install library" OFF)



if(ELANOR_BUILD_UNIT_TESTS)
	message("UnitTest enabled for ElanorBot")
	find_package(GTest QUIET)
	if (NOT GTest_FOUND)
		message("GooogleTest not found, building from subdirectory")
		add_subdirectory(googletest)
	endif()
endif(ELANOR_BUILD_UNIT_TESTS)

if(ELANOR_INSTALL)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install library in the current directory" FORCE)
endif(ELANOR_INSTALL)



# Set logging level to DEBUG
add_compile_definitions(MIRAI_LOGGING_LEVEL=1)

# add_subdirectory(ElanorApp)
add_subdirectory(ElanorCore)
# add_subdirectory(Plugins)