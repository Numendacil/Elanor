cmake_minimum_required (VERSION 3.20)

project (ElanorCore VERSION 1.0.0 LANGUAGES CXX)

# Library names
set(ELANORBOT_CORE ElanorCore)
set(ELANORBOT_MOCKCORE ElanorCoreMock)

# ELanorCore
add_library(${ELANORBOT_CORE} SHARED)
add_library(${CMAKE_PROJECT_NAME}::${ELANORBOT_CORE} ALIAS ${ELANORBOT_CORE})

target_include_directories(
	${ELANORBOT_CORE} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(${ELANORBOT_CORE} PUBLIC cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(${ELANORBOT_CORE} PUBLIC Threads::Threads)

set_target_properties(${ELANORBOT_CORE} PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")
set_target_properties(${ELANORBOT_CORE} PROPERTIES PREFIX "")

# For building shared libs
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Mock libraries
option(ELANOR_BUILD_MOCK_LIB "Build MockLibs" OFF)
if(ELANOR_BUILD_MOCK_LIB)
	message("Build ElanorCoreMock")
	
	add_library(${ELANORBOT_MOCKCORE})
	add_library(${CMAKE_PROJECT_NAME}::${ELANORBOT_MOCKCORE} ALIAS ${ELANORBOT_MOCKCORE})
	target_compile_features(${ELANORBOT_MOCKCORE} PUBLIC cxx_std_20)
	target_link_libraries(${ELANORBOT_MOCKCORE} PUBLIC Threads::Threads)
	target_include_directories(${ELANORBOT_MOCKCORE} PUBLIC .)

endif(ELANOR_BUILD_MOCK_LIB)

set(MIRAI_INSTALL ON CACHE BOOL "Install library" FORCE)

add_subdirectory(Core/Bot)
add_subdirectory(Core/Client)
add_subdirectory(Core/Interface)
add_subdirectory(Core/States)
add_subdirectory(Core/Utils)

if(ELANOR_BUILD_MOCK_LIB)
	
	get_target_property(SOURCE_LIST ${ELANORBOT_CORE} SOURCES)
	target_sources(${ELANORBOT_MOCKCORE} PRIVATE ${SOURCE_LIST})
	target_link_libraries(${ELANORBOT_MOCKCORE} PUBLIC cpp-mirai-client::cppmiraiMock)

endif(ELANOR_BUILD_MOCK_LIB)

# UnitTests
if(ELANOR_BUILD_UNIT_TESTS)
	add_subdirectory(UnitTest)
endif(ELANOR_BUILD_UNIT_TESTS)

# Installation
install(
	TARGETS ${ELANORBOT_CORE}
	EXPORT ${ELANORBOT_CORE}-targets
	DESTINATION "lib"
)

install(
	TARGETS ${ELANORBOT_CORE}
	DESTINATION "bin"
)

install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
	DESTINATION "include"
	FILES_MATCHING
	PATTERN "*.hpp"
	PATTERN "cpp-mirai-client" EXCLUDE
)

configure_file("${ELANORBOT_CORE}-config.cmake.in" "${ELANORBOT_CORE}-config.cmake" @ONLY)
install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/${ELANORBOT_CORE}-config.cmake"
	DESTINATION "lib/cmake/${ELANORBOT_CORE}"
)

install(
	EXPORT ${ELANORBOT_CORE}-targets 
	FILE "${ELANORBOT_CORE}-targets.cmake"
	NAMESPACE ${PROJECT_NAME}::
	DESTINATION "lib/cmake/${ELANORBOT_CORE}"
)