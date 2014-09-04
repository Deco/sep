set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/serial_port_bsd.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/serial_port_bsd.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/serial_port_win.cpp
        PARENT_SCOPE
    )
endif()
