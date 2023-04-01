target_sources(
	${ELANORBOT_CORE} PRIVATE
	Client.hpp
	Client.cpp
)

add_subdirectory(cpp-mirai-client)

target_link_libraries(${ELANORBOT_CORE} PUBLIC cpp-mirai-client::cppmirai)

if(BUILD_SHARED_LIBS)
	install(
		TARGETS cppmirai
		DESTINATION "bin"
	)
endif(BUILD_SHARED_LIBS)