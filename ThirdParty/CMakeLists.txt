add_library(ThirdParty INTERFACE)
add_library(${ELANORBOT_TARGET_NAME}::ThirdParty ALIAS ThirdParty)

target_link_libraries(ThirdParty INTERFACE httplib::httplib)