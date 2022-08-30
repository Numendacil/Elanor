cmake_minimum_required (VERSION 3.20)

project(ElanorBot LANGUAGES CXX VERSION 1.0.0)

set(ELANORBOT_TARGET_NAME ElanorBot)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${ELANORBOT_TARGET_NAME})

target_include_directories(${ELANORBOT_TARGET_NAME} PRIVATE .)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE ZLIB::ZLIB)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE OpenSSL::SSL)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE OpenSSL::Crypto)
target_compile_features(${ELANORBOT_TARGET_NAME} PRIVATE cxx_std_20)
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE pthread)
# 静态链接 libgcc 和 libstdc++
target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE -static-libgcc -static-libstdc++)

target_link_libraries(${ELANORBOT_TARGET_NAME} PRIVATE -static-libasan)
target_compile_options(${ELANORBOT_TARGET_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(${ELANORBOT_TARGET_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)

add_subdirectory(App)
add_subdirectory(Client)
add_subdirectory(Group)
add_subdirectory(Command)
add_subdirectory(State)
add_subdirectory(Trigger)
add_subdirectory(ThirdParty)
add_subdirectory(Utils)