set(ELANORBOT_CORE ElanorCore)
add_library(${ELANORBOT_CORE})
add_library(${CMAKE_PROJECT_NAME}::${ELANORBOT_CORE} ALIAS ${ELANORBOT_CORE})

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

target_include_directories(${ELANORBOT_CORE} PUBLIC .)

target_link_libraries(${ELANORBOT_CORE} PUBLIC ZLIB::ZLIB)
target_link_libraries(${ELANORBOT_CORE} PUBLIC OpenSSL::SSL)
target_link_libraries(${ELANORBOT_CORE} PUBLIC OpenSSL::Crypto)

target_compile_features(${ELANORBOT_CORE} PUBLIC cxx_std_20)
target_link_libraries(${ELANORBOT_CORE} PUBLIC pthread)

add_subdirectory(Core/Bot)
add_subdirectory(Core/Client)
add_subdirectory(Core/Interface)
add_subdirectory(Core/States)
add_subdirectory(Core/Utils)