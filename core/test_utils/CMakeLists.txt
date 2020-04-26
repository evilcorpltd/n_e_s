include(FetchContent)
include(GoogleTest)

project(n_e_s_core_test_utils)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        e3f0319d89f4cbf32993de595d984183b1a9fc57 # 1.10 + build fix for clang on Windows
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(${PROJECT_NAME} INTERFACE)
add_library(n_e_s::core::test_utils ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        include
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        INTERFACE_COMPILE_FEATURES cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        n_e_s::core
        n_e_s::warnings
        gmock
)

