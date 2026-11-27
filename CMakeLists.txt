cmake_minimum_required(VERSION 3.28.3)
project(p2p_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(peer
    src/main.cpp
    src/common/socket_io.cpp
    src/common/protocol.cpp
)

add_executable(socket_io_test
    tests/socket_io_test.cpp
    src/common/socket_io.cpp
)

target_include_directories(socket_io_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(protocol_test
    tests/protocol_test.cpp
    src/common/protocol.cpp
)

target_include_directories(protocol_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Threads REQUIRED)
target_link_libraries(peer PRIVATE Threads::Threads)