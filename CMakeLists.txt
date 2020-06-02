cmake_minimum_required (VERSION 2.8)

Project(library_curation NONE)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_HOME_DIRECTORY}/dist")

if (NOT DEFINED REGISTRY)
    set(REGISTRY "")
elseif (NOT ${REGISTRY} MATCHES "/$")
    set(REGISTRY "${REGISTRY}/")
endif()
if (NOT DEFINED PLATFORM)
    set(PLATFORM "Xeon")
endif()
if (NOT DEFINED NCURATIONS)
    set(NCURATIONS 1)
endif()
if (NOT DEFINED INGESTION)
    set(INGESTION "object,face")
endif()

if(NOT REGISTRY)
    add_custom_target(update  ${CMAKE_HOME_DIRECTORY}/script/update-image.sh)
endif()
add_custom_target(dist  ${CMAKE_HOME_DIRECTORY}/script/mk-dist.sh)

file(GLOB dirs "deployment" "*")
list(REMOVE_DUPLICATES dirs)
foreach (dir ${dirs})
    if(EXISTS ${dir}/CMakeLists.txt)
        add_subdirectory(${dir})
    endif()
endforeach()

# legal message
execute_process(COMMAND printf "\n\nThis script will build third party components licensed under various open source licenses into your container images. The terms under which those components may be used and distributed can be found with the license document that is provided with those components. Please familiarize yourself with those terms to ensure your distribution of those components complies with the terms of those licenses.\n\n")

execute_process(COMMAND printf "PLATFORM=${PLATFORM}, NCURATIONS=${NCURATIONS}, INGESTION=${INGESTION}\n")
execute_process(COMMAND printf "REGISTRY=${REGISTRY}\n\n")

