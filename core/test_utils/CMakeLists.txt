include(GoogleTest)

project(n_e_s_core_test_utils)

add_library(${PROJECT_NAME} INTERFACE)
add_library(n_e_s::core::test_utils ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        include
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        INTERFACE_COMPILE_FEATURES cxx_std_20
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        n_e_s::core
        n_e_s::warnings
        gmock
)

