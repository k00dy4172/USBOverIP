cmake_minimum_required(VERSION 3.20)

project(USBOverIP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ============================================================
# Client
# ============================================================

add_executable(
    USBOverIPClient

    src/main_client.cpp
    src/USBPcapTransport.cpp
    src/NetworkTransport.cpp
)


target_include_directories(
    USBOverIPClient
    PRIVATE
        include
        external/libusb/include
)


target_link_directories(
    USBOverIPClient
    PRIVATE
        external/libusb/static
)


target_link_libraries(
    USBOverIPClient
    PRIVATE
        libusb-1.0
        ws2_32
)


# ============================================================
# Server
# ============================================================

add_executable(
    USBOverIPServer

    src/main_server.cpp
    src/NetworkTransport.cpp
)


target_include_directories(
    USBOverIPServer
    PRIVATE
        include
)


target_link_libraries(
    USBOverIPServer
    PRIVATE
        ws2_32
)