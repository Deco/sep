cmake_minimum_required(VERSION 2.8)

# define our project!
project(scanning_demo)

# WARNING: DEBUG BUILD
message(STATUS "WARNING: DEBUG BUILD")
set(CMAKE_CXX_FLAGS "-ggdb -O0")

# ensure the compiler is using C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

# find the repository root, so we can get to components and dependencies
if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)
endif()

# for configuration headers, add this target's source directory to the include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/source")

# define where executables, libraries, data, etc will be placed
set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/output)

message(STATUS "test: " ${CMAKE_CURRENT_BINARY_DIR})

# this is where data files will be (UI resources, images, etc)
set(OUTPUT_RESOURCE_DIR ${OUTPUT_DIR}/res)

# Specify each module needed by this target here
# CMake will recurse into their folders and execute their CMakeLists.txt
# Those will do the following:
#  - Add their directories to INCLUDE_PATHS
#  - Add their source files to SOURCE_FILES
#  - Add linking libraries to LINK_PATHS
#  - Copy their data files to OUTPUT_DATA_DIR
#
# Note: components will not add other components!
#       You must add their dependencies for them.
#
function(target_module DIRECTORY)
    add_subdirectory("${CMAKE_PROJECT_ROOT}/${DIRECTORY}" "${CMAKE_CURRENT_BINARY_DIR}/${DIRECTORY}")
endfunction()

# External dependencies
target_module(dependencies/libevent )
target_module(dependencies/msgpack-c)

# Internal components
#target_module(components/thermal_sensor_controller)
#target_module(components/actuator_controller      )


# add the include directories
#include_directories("${INCLUDE_PATHS}")

# add the source files of this target
#set(SOURCE_FILES
#    ${SOURCE_FILES}
#    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
#)

# specify where the output executables and libraries will be placed
#set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})
#set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR})

# add the executable
#add_executable(${PROJECT_NAME}
#    ${SOURCE_FILES}
#)

# inform the executable of the linking paths
#target_link_libraries(${PROJECT_NAME} LINK_PATHS)

# done!
