cmake_minimum_required(VERSION 3.20)

project(USBOverIP)

add_executable(
    USBOverIP
    src/main.cpp
    src/hello.cpp
    src/USBManager.cpp
    src/RawInputManager.cpp
)

target_include_directories(
    USBOverIP
    PRIVATE
        include
        external/libusb/include
)

target_link_directories(
    USBOverIP
    PRIVATE
        external/libusb/static
)

target_link_libraries(
    USBOverIP
    PRIVATE
        libusb-1.0
)

add_custom_command(
    TARGET USBOverIP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/external/libusb/bin/libusb-1.0.dll
        $<TARGET_FILE_DIR:USBOverIP>
)

set_property(TARGET USBOverIP PROPERTY CXX_STANDARD 20)