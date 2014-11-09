cmake_minimum_required(VERSION 2.8)

# Include paths
get_property(INCLUDE_PATHS GLOBAL PROPERTY PROJECT_INCLUDE_PATHS)
set(INCLUDE_PATHS
    ${INCLUDE_PATHS}
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
set_property(GLOBAL PROPERTY PROJECT_INCLUDE_PATHS ${INCLUDE_PATHS})

# Source files
get_property(SOURCE_FILES GLOBAL PROPERTY PROJECT_SOURCE_FILES)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/serial_port_bsd.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/serial_port_bsd.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/serial_port_win.cpp
    )
endif()

set_property(GLOBAL PROPERTY PROJECT_SOURCE_FILES ${SOURCE_FILES})

