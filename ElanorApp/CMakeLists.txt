cmake_minimum_required (VERSION 3.20)

project(ElanorApp LANGUAGES CXX VERSION 1.0.0)

set(ELANORBOT_APP ElanorApp)

add_executable(${ELANORBOT_APP})
add_executable(${CMAKE_PROJECT_NAME}::${ELANORBOT_APP} ALIAS ${ELANORBOT_APP})
set_target_properties(${ELANORBOT_APP} PROPERTIES OUTPUT_NAME "ElanorBot")

target_sources(
	${ELANORBOT_APP} PRIVATE
	ElanorBot.hpp
	ElanorBot.cpp
	main.cpp
)

target_include_directories(${ELANORBOT_APP} PRIVATE .)
target_link_libraries(${ELANORBOT_APP} PRIVATE ${CMAKE_PROJECT_NAME}::ElanorCore ${CMAKE_DL_LIBS})

add_subdirectory(Utils)

set_target_properties(${ELANORBOT_APP} PROPERTIES INSTALL_RPATH "$\{ORIGIN\};${INSTALL_RPATH}")
install(
	TARGETS ${ELANORBOT_APP}
	DESTINATION "bin"
)