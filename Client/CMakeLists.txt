
add_library(Client STATIC
		Client.hpp
		Client.cpp)

add_subdirectory(cpp-mirai-client)
target_link_libraries(Client PUBLIC cpp-mirai-client::cppmirai)
