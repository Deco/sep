
cmake_minimum_required(VERSION 2.6) 
project(trackingthingo)

add_definitions(-ggdb -O0)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0 -ggdb")

FILE(GLOB ProjectSources *.cpp)

add_executable(trackingthingo ${ProjectSources})

if(UNIX)
  target_link_libraries(trackingthingo pthread ${Boost_LIBRARIES})

  find_package( Boost 1.40 COMPONENTS system program_options REQUIRED )
  include_directories( ${Boost_INCLUDE_DIR} )
  target_link_libraries(trackingthingo
    ${Boost_SYSTEM_LIBRARY}
  )
endif(UNIX)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "cmake_modules/")
find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(trackingthingo ${SFML_LIBRARIES})
endif()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( trackingthingo ${OpenCV_LIBS} )

install(TARGETS trackingthingo DESTINATION bin)


