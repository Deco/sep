#set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(SOURCE_FILES
   # ${SOURCE_FILES}
  #  ${CMAKE_CURRENT_SOURCE_DIR}/sensor_conntroller.cpp
 #   PARENT_SCOPE
#)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})


cmake_minimum_required(VERSION 2.8)


#IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    add_definitions(-DSERIALCONN_USE_IKARUS=1)
#else(UNIX)
    add_definitions(-DSERIALCONN_USE_BOOST=1)
#endif()



if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT
        ${CMAKE_CURRENT_SOURCE_DIR}/../../
    )
endif()


set(CMAKE_CXX_FLAGS "-std=c++11 -ggdb")


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")


FIND_PACKAGE( Boost 1.40 COMPONENTS system REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set (serialconn_SOURCE ${CMAKE_PROJECT_ROOT}components/thermal_sensor_controller/)
set (thermal_controller_SOURCE ${CMAKE_PROJECT_ROOT}components/thermal_sensor_controller/)

message(serialconn_SOURCE: ${serialconn_SOURCE})

add_executable(main
    #main.cpp
    #serialconn 
    #sensor_controller.cpp
    main
    ${serialconn_SOURCE}/serialconn.h
    ${thermal_controller_SOURCE}/serialconn.cpp
    ${serialconn_SOURCE}/sensor_controller.h
    ${thermal_controller_SOURCE}/sensor_controller.cpp
)

TARGET_LINK_LIBRARIES( main ${Boost_LIBRARIES} )

target_link_libraries( main ${OpenCV_LIBS} )
