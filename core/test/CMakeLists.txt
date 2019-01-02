include(FetchContent)
include(GoogleTest)

project(test_n_e_s_core)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        7515e399436a832a0109d64a70b4e1125568dda5
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mock_membank.h
    src/mock_mmu.h
    src/mock_ppu.h
    src/test_cpu.cpp
    src/test_invalid_address.cpp
    src/test_mmu.cpp
    src/test_ppu.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME}
        PRIVATE
           -Wall
           -Werror
           -Wextra
           -Wswitch-enum
           -Wshadow
           -Wnon-virtual-dtor
           -pedantic-errors
    )
endif()

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s_core
        gmock
)

gtest_discover_tests(${PROJECT_NAME})
