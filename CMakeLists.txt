cmake_minimum_required(VERSION 3.20)

project(USBOverIP)

add_executable(USBOverIP
    src/main.cpp
    src/hello.cpp
)

target_include_directories(
    USBOverIP
    PRIVATE
        include
)

set_property(TARGET USBOverIP PROPERTY CXX_STANDARD 20)