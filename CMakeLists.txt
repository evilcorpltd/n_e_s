cmake_minimum_required(VERSION 3.11)

project(n_e_s)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(sanitizers)
include(gtest)

enable_testing()

add_subdirectory(application)
add_subdirectory(core)
add_subdirectory(nes)
