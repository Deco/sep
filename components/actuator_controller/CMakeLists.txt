set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
    ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/actuator_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actuator_comm_ax12.cpp
    PARENT_SCOPE
)

