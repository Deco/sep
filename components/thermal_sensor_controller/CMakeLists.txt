cmake_minimum_required(VERSION 2.8)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-SERIALCONN_USE_IKARUS=1)
else(UNIX)
    add_definitions(-DSERIALCONN_USE_BOOST=1)
endif()



if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../
    )
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -ggdb")


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")


FIND_PACKAGE( Boost 1.40 COMPONENTS system REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(main
    main.cpp
    serialconn.cpp
    sensor_controller.cpp
)

TARGET_LINK_LIBRARIES( main ${Boost_LIBRARIES} )

target_link_libraries( main ${OpenCV_LIBS} )


