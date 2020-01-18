cmake_minimum_required(VERSION 3.14)

project(n_e_s)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(sanitizers)

enable_testing()

add_subdirectory(warnings)

add_subdirectory(application)
add_subdirectory(core)
add_subdirectory(nes)
add_subdirectory(nestest)
