cmake_minimum_required(VERSION 2.8)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
    ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/actuator_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actuator_comm_ax12.cpp
    PARENT_SCOPE
)

set(INCLUDE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/
    PARENT_SCOPE
)
