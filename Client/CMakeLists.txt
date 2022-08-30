target_sources(${ELANORBOT_TARGET_NAME} PRIVATE
		Client.hpp
		Client.cpp)

add_subdirectory(cpp-mirai-client)
target_link_libraries(${ELANORBOT_TARGET_NAME} PUBLIC cpp-mirai-client::cppmirai)