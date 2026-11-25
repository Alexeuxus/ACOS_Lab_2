cmake_minimum_required(VERSION 3.28.3)
project(p2p_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(peer
    src/main.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(peer PRIVATE Threads::Threads)