target_sources(
	${ELANORBOT_CORE} PRIVATE
	Client.hpp
	Client.cpp
)
if(ELANOR_BUILD_MOCK_LIB)
	set(MIRAI_BUILD_MOCK_CLIENT ON CACHE BOOL "Build MockLibs" FORCE)
endif(ELANOR_BUILD_MOCK_LIB)

set(MIRAI_BUILD_SHARED_LIBS ON CACHE BOOL "Build ${MIRAI_LIB_TARGET_NAME} as a shared library." FORCE)
add_subdirectory(cpp-mirai-client)
target_link_libraries(${ELANORBOT_CORE} PUBLIC cpp-mirai-client::cppmirai)

install(
	TARGETS cppmirai
	DESTINATION "bin"
)