set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
    ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_conntroller.cpp
    PARENT_SCOPE
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
