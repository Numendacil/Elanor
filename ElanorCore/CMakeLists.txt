cmake_minimum_required (VERSION 3.20)

project (ElanorCore VERSION 1.0.0 LANGUAGES CXX)

set(ELANORBOT_CORE ElanorCore)

add_library(${ELANORBOT_CORE} SHARED)
add_library(${CMAKE_PROJECT_NAME}::${ELANORBOT_CORE} ALIAS ${ELANORBOT_CORE})

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

target_include_directories(
	${ELANORBOT_CORE} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
	$<INSTALL_INTERFACE:include>
)

target_link_libraries(${ELANORBOT_CORE} PUBLIC ZLIB::ZLIB)
target_link_libraries(${ELANORBOT_CORE} PUBLIC OpenSSL::SSL)
target_link_libraries(${ELANORBOT_CORE} PUBLIC OpenSSL::Crypto)

target_compile_features(${ELANORBOT_CORE} PUBLIC cxx_std_20)
target_link_libraries(${ELANORBOT_CORE} PUBLIC pthread)

set_target_properties(${ELANORBOT_CORE} PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install library in the current directory" FORCE)

add_subdirectory(Core/Bot)
add_subdirectory(Core/Client)
add_subdirectory(Core/Interface)
add_subdirectory(Core/States)
add_subdirectory(Core/Utils)

install(
	TARGETS ${ELANORBOT_CORE}
	EXPORT ${ELANORBOT_CORE}-targets
	DESTINATION "lib"
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