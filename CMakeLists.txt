cmake_minimum_required(VERSION 3.14)

project(n_e_s)

include(cmake/conan.cmake)
include(cmake/sanitizers.cmake)

run_conan()

enable_testing()

add_subdirectory(warnings)

add_subdirectory(application)
add_subdirectory(core)
add_subdirectory(nes)
