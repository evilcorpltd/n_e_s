include(GoogleTest)

project(test_n_e_s_disassembler)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/test_disassembler.cpp
)

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
        n_e_s::core
        n_e_s::core::test_utils
        n_e_s::disassembler
        n_e_s::warnings
        gmock
)

gtest_discover_tests(${PROJECT_NAME})

