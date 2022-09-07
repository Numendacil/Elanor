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
target_link_libraries(${ELANORBOT_APP} PRIVATE ${CMAKE_PROJECT_NAME}::ElanorCore)
