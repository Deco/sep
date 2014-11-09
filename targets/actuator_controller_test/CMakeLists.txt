
# External dependencies
target_module(dependencies/boost)

# Internal components
target_module(components/util               )
target_module(components/core               )
target_module(components/serial_port        )
target_module(components/actuator_controller)


# Include paths
get_property(INCLUDE_PATHS GLOBAL PROPERTY PROJECT_INCLUDE_PATHS)
set(INCLUDE_PATHS
    ${INCLUDE_PATHS}
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/
)
set_property(GLOBAL PROPERTY PROJECT_INCLUDE_PATHS ${INCLUDE_PATHS})

# Source files
get_property(SOURCE_FILES GLOBAL PROPERTY PROJECT_SOURCE_FILES)
set(SOURCE_FILES
    ${SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp
)
set_property(GLOBAL PROPERTY PROJECT_SOURCE_FILES ${SOURCE_FILES})

