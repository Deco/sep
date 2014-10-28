cmake_minimum_required(VERSION 2.8)

# define our project!
project(actuator_controller_test)


# WARNING: DEBUG BUILD
message(STATUS "WARNING: DEBUG BUILD")
set(CMAKE_CXX_FLAGS "-ggdb -O0")

# ensure the compiler is using C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# find the repository root, so we can get to components and dependencies
if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)
endif()

# for configuration headers, add this target's source directory to the include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/source")

# define where executables, libraries, data, etc will be placed
set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/output)

# this is where data files will be (UI resources, images, etc)
set(OUTPUT_RESOURCE_DIR ${OUTPUT_DIR}/res)

# Source files and include paths lists
set(SOURCE_FILES "")
set(INCLUDE_PATHS "")
set(LINK_PATHS "")

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
target_module(dependencies/boost)

# Internal components
target_module(components/util               )
target_module(components/core               )
target_module(components/serial_port        )
target_module(components/actuator_controller)

# add the include directories
#include_directories("${INCLUDE_PATHS}")
include_directories(
	"${CMAKE_PROJECT_ROOT}/components/util" 
	"${CMAKE_PROJECT_ROOT}/components/core"       
	"${CMAKE_PROJECT_ROOT}/components/serial_port"
	"${CMAKE_PROJECT_ROOT}/components/actuator_controller"
)


# add the source files of this target
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp
    ${CMAKE_PROJECT_ROOT}/components/actuator_controller/actuator_comm.cpp
    ${CMAKE_PROJECT_ROOT}/components/actuator_controller/actuator_comm_ax12.cpp
    ${CMAKE_PROJECT_ROOT}/components/actuator_controller/actuator_controller.cpp
    ${CMAKE_PROJECT_ROOT}/components/serial_port/serial_port_bsd.cpp
    ${CMAKE_PROJECT_ROOT}/components/core/application_core.cpp
)

# specify where the output executables and libraries will be placed
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR})

find_package(Boost) 
find_package(Boost 1.45 COMPONENTS program_options system REQUIRED)

# add the executable
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS}) 
	add_executable(actuator_controller_test
	    ${SOURCE_FILES}
	)
	target_link_libraries(actuator_controller_test ${Boost_LIBRARIES})   
endif()


# done!