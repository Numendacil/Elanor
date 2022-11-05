add_library(cricpp)

add_subdirectory(cricpp)

target_compile_features(cricpp PRIVATE cxx_std_20)
target_include_directories(cricpp PUBLIC .)