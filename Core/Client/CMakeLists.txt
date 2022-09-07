target_sources(
	${ELANORBOT_CORE} PRIVATE
	Client.hpp
	Client.cpp
)

add_subdirectory(cpp-mirai-client)
target_link_libraries(${ELANORBOT_CORE} PUBLIC cpp-mirai-client::cppmirai)