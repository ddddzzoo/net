cmake_minimum_required(VERSION 3.20)
project(net_learn)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(socket)
add_subdirectory(system)