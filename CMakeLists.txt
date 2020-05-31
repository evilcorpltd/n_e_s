cmake_minimum_required(VERSION 3.14)

project(n_e_s)

include(cmake/sanitizers.cmake)
include(cmake/coverage.cmake)

enable_testing()

add_subdirectory(third_party)
add_subdirectory(warnings)

add_subdirectory(application)
add_subdirectory(core)
add_subdirectory(disassembler)
add_subdirectory(nes)
add_subdirectory(nestest)
