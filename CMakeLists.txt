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
    src/inbound/listener.cpp
    src/inbound/inbound_fsm.cpp
    src/inbound/connection_loop.cpp
    src/common/socket_writer.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(peer PRIVATE Threads::Threads)

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

add_executable(listener_test
    tests/listener_test.cpp
    src/inbound/listener.cpp
    src/common/socket_io.cpp
)

target_include_directories(listener_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(inbound_fsm_test
    tests/inbound_fsm_test.cpp
    src/inbound/inbound_fsm.cpp
    src/common/socket_io.cpp
    src/common/protocol.cpp
    src/common/socket_writer.cpp
)

target_include_directories(inbound_fsm_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(inbound_fsm_test PRIVATE Threads::Threads)

add_executable(connection_loop_test
    tests/connection_loop_test.cpp
    src/inbound/connection_loop.cpp
    src/inbound/inbound_fsm.cpp
    src/common/socket_io.cpp
    src/common/protocol.cpp
    src/common/socket_writer.cpp
)

target_include_directories(connection_loop_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(connection_loop_test PRIVATE Threads::Threads)

add_executable(socket_writer_test
    tests/socket_writer_test.cpp
    src/common/socket_writer.cpp
    src/common/socket_io.cpp
)

target_include_directories(socket_writer_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(socket_writer_test PRIVATE
    Threads::Threads
)