add_library(cppcodec INTERFACE)
target_include_directories(cppcodec INTERFACE cppcodec/)
target_link_libraries( ${ELANORBOT_TARGET_NAME} PRIVATE cppcodec)

target_sources( ${ELANORBOT_TARGET_NAME} 
	PRIVATE 
		croncpp.h
		date.h
		log.h
		uuid.h
	)

target_link_libraries( ${ELANORBOT_TARGET_NAME} PRIVATE httplib::httplib)