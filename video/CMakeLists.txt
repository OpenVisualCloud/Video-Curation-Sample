set(service "lcc_video")
include("${CMAKE_SOURCE_DIR}/script/service.cmake")

# add stream to the clean list
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/archive") 
