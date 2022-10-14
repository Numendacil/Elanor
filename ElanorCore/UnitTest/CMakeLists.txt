cmake_minimum_required(VERSION 3.20)
project(ElanorCoreTest)

message("ElanorCoreTest enabled")

add_executable(
	ElanorCoreTest
	
	StatesTest.cpp
)

target_link_libraries(ElanorCoreTest PRIVATE ${ELANORBOT_CORE})
target_link_libraries(ElanorCoreTest PRIVATE GoogleTestLibs)

gtest_discover_tests(ElanorCoreTest DISCOVERY_TIMEOUT 300)