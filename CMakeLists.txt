cmake_minimum_required (VERSION 3.20)

project(ElanorBot LANGUAGES CXX VERSION 1.0.0)

set(ELANORBOT_TARGET_NAME ElanorBot)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${ELANORBOT_TARGET_NAME})

target_include_directories(${ELANORBOT_TARGET_NAME} PRIVATE .)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE Client)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE ZLIB::ZLIB)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE OpenSSL::SSL)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE OpenSSL::Crypto)
target_compile_features(${ELANORBOT_TARGET_NAME} PRIVATE cxx_std_20)
set_property(TARGET ${ELANORBOT_TARGET_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${ELANORBOT_TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(App)
add_subdirectory(Client)
add_subdirectory(Group)
add_subdirectory(Command)
add_subdirectory(State)
add_subdirectory(Trigger)
add_subdirectory(ThirdParty)
add_subdirectory(Utils)