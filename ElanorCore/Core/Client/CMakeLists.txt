target_sources(
	${ELANORBOT_CORE} PRIVATE
	Client.hpp
	Client.cpp
)
if(ELANOR_BUILD_MOCK_LIB)
	set(MIRAI_BUILD_MOCK_CLIENT ON CACHE BOOL "Build MockLibs" FORCE)
endif(ELANOR_BUILD_MOCK_LIB)

add_subdirectory(cpp-mirai-client)
target_link_libraries(${ELANORBOT_CORE} PUBLIC cpp-mirai-client::cppmirai)