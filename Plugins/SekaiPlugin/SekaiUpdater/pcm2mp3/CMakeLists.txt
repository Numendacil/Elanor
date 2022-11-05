add_library(pcm2mp3)

target_sources(
	pcm2mp3
	PRIVATE

	pcm2mp3/encode.hpp
	pcm2mp3/encode.cpp
)

target_include_directories(pcm2mp3 PUBLIC .)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET
	libavformat
	libavcodec
	libavutil
)

target_link_libraries(pcm2mp3 PRIVATE PkgConfig::FFmpeg)
target_compile_features(pcm2mp3 PRIVATE cxx_std_20)