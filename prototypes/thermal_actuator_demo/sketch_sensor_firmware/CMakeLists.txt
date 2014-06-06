cmake_minimum_required(VERSION 2.6) 

if(NOT DEFINED CMAKE_PROJECT_ROOT)
    set(CMAKE_PROJECT_ROOT
        ${CMAKE_SOURCE_DIR}/../../../
    )
endif()

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_PROJECT_ROOT}/dependencies/arduino-cmake/cmake/ArduinoToolchain.cmake
)

message(STATUS ${CMAKE_TOOLCHAIN_FILE})

project(sketch_sensor_firmware)

if(DEFINED SKETCH_SENSOR_FIRMWARE_PORT)
    set(${FIRMWARE_NAME}_PORT
        SKETCH_SENSOR_FIRMWARE_PORT
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(${FIRMWARE_NAME}_PORT
        "/dev/tty.usbmodem1421"
    )
else()
    message(SEND_ERROR
        "Could not auto-detect your Arduino. Please set -DSKETCH_SENSOR_FIRMWARE_PORT=/dev/name_of_device"
    )
endif()
message(STATUS
    "Arduino port name: ${SKETCH_SENSOR_FIRMWARE_PORT}"
)

set(sketch_sensor_firmware_SKETCH
    ${CMAKE_SOURCE_DIR}
)
set(sketch_sensor_firmware_BOARD micro)

generate_arduino_firmware(sketch_sensor_firmware)

