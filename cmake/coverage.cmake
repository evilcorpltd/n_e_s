if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
      OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   option(ENABLE_COVERAGE "Enable coverage" OFF)

   if (ENABLE_COVERAGE)
      message(STATUS "Enabling code coverage")
      add_compile_options(--coverage)
      link_libraries(--coverage)
   endif()
endif()
