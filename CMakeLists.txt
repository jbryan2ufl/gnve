cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 4.1.1)
project(GNVEApp VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/dependencies.cmake)

add_subdirectory(assets)
add_subdirectory(shaders)
add_subdirectory(src)
