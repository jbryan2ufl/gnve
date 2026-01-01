cmake_minimum_required(VERSION 3.21)
project(GNVEApp VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/dependencies.cmake)

add_subdirectory(assets)
add_subdirectory(shaders)
add_subdirectory(src)
