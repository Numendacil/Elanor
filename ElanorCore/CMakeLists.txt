cmake_minimum_required (VERSION 3.20)

project (ElanorCore VERSION 1.0.0 LANGUAGES CXX)

# Library names
set(ELANORBOT_CORE ElanorCore)

# ELanorCore
add_library(${ELANORBOT_CORE})
add_library(${CMAKE_PROJECT_NAME}::${ELANORBOT_CORE} ALIAS ${ELANORBOT_CORE})

target_include_directories(
	${ELANORBOT_CORE} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(${ELANORBOT_CORE} PUBLIC cxx_std_20)

add_subdirectory(Core/Bot)
add_subdirectory(Core/Client)
add_subdirectory(Core/Interface)
add_subdirectory(Core/Utils)

# UnitTests
if(ELANOR_BUILD_UNIT_TESTS)
	add_subdirectory(UnitTest)
endif(ELANOR_BUILD_UNIT_TESTS)

# Install

install(
	TARGETS ${ELANORBOT_CORE}
	EXPORT ${ELANORBOT_CORE}-targets
	DESTINATION "lib"
)


if(BUILD_SHARED_LIBS)
	set_target_properties(${ELANORBOT_CORE} PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")
	set_target_properties(${ELANORBOT_CORE} PROPERTIES PREFIX "")

	install(
		TARGETS ${ELANORBOT_CORE}
		DESTINATION "bin"
	)
endif(BUILD_SHARED_LIBS)

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