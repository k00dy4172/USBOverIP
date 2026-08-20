cmake_minimum_required(VERSION 3.20)

project(USBOverIP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ========================================
# Client
# ========================================

add_executable(USBOverIPClient
    src/main_client.cpp
    src/RawInputTransport.cpp
    src/NetworkTransport.cpp
)

target_include_directories(
    USBOverIPClient
    PRIVATE
        include
)

target_link_libraries(
    USBOverIPClient
    PRIVATE
        ws2_32
)


# ========================================
# Server
# ========================================

add_executable(USBOverIPServer
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
add_custom_command(
    TARGET USBOverIPClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/external/libusb/bin/libusb-1.0.dll
        $<TARGET_FILE_DIR:USBOverIPClient>
)

set_property(TARGET USBOverIPClient PROPERTY CXX_STANDARD 20)