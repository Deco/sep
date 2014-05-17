cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../
    )
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Dynamixel")

FILE(GLOB DYNAMIXEL_SOURCES "Dynamixel/*.cc")

add_executable(main
    main.cpp
    ${DYNAMIXEL_SOURCES}
)
