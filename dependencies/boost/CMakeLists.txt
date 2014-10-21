cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.45 COMPONENTS program_options REQUIRED)

set(INCLUDE_PATHS
    ${Boost_INCLUDE_DIR}
    PARENT_SCOPE
)

set(LINK_PATHS
    ${Boost_LIBRARIES}
    PARENT_SCOPE
)
