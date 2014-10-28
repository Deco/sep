
cmake_minimum_required(VERSION 2.8)

# define our project!
project(cts)

if(NOT DEFINED TARGET)
    message(FATAL_ERROR "Please defined build target using 'cmake -DTARGET=target_name ...'.")
endif()

# find the repository root, so we can get to components and dependencies
if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# ensure the compiler is using C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# include common CMake file
include("${CMAKE_PROJECT_ROOT}/targets/common/CMakeLists.txt")

# define where executables, libraries, data, etc will be placed
set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/output)

# this is where data files will be (UI resources, images, etc)
set(OUTPUT_RESOURCE_DIR ${OUTPUT_DIR}/res)

# Source files and include paths lists
set(SOURCE_FILES "")
set(INCLUDE_PATHS "")
set(LINK_PATHS "")

set_property(GLOBAL PROPERTY PROJECT_SOURCE_FILES ${SOURCE_FILES})
set_property(GLOBAL PROPERTY PROJECT_INCLUDE_PATHS ${INCLUDE_PATHS})
set_property(GLOBAL PROPERTY PROJECT_LINK_PATHS ${LINK_PATHS})

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

# Include target
add_subdirectory("${CMAKE_PROJECT_ROOT}/targets/${TARGET}/")

# Retrieve properties
get_property(SOURCE_FILES GLOBAL PROPERTY PROJECT_SOURCE_FILES)
get_property(INCLUDE_PATHS GLOBAL PROPERTY PROJECT_INCLUDE_PATHS)
get_property(LINK_PATHS GLOBAL PROPERTY PROJECT_LINK_PATHS)

# add the include directories
include_directories("${INCLUDE_PATHS}")

# 
message(STATUS "target: ${TARGET}")
foreach(path ${SOURCE_FILES})
  message(STATUS "source: ${path}")
endforeach()
foreach(path ${INCLUDE_PATHS})
  message(STATUS "include: ${path}")
endforeach()
foreach(path ${LINK_PATHS})
  message(STATUS "link: ${path}")
endforeach()

# specify where the output executables and libraries will be placed
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR})

# add the executable
add_executable(${TARGET}
    ${SOURCE_FILES}
)
target_link_libraries(${TARGET} ${LINK_PATHS})


# done!



