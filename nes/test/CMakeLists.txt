include(GoogleTest)

project(test_n_e_s_nes)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/test_nes.cpp
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::nes
        n_e_s::warnings
        gmock
)

gtest_discover_tests(${PROJECT_NAME})
