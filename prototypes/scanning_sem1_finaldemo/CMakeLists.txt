cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT
        ${CMAKE_CURRENT_SOURCE_DIR}/../../
    )
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -ggdb")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Dynamixel")
FILE(GLOB DYNAMIXEL_SOURCES "Dynamixel/*.cc")

add_executable(main
    main.cpp

    SensorSamplerRealm.cpp

    ActuatorController.cpp
    ${DYNAMIXEL_SOURCES}

    sensor_controller.cpp
    serialconn.cpp
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(main ${SFML_LIBRARIES})
endif()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( main ${OpenCV_LIBS} )
