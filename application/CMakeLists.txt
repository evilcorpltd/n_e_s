project(n_e_s_application)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::nes
        n_e_s::warnings
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

#####################################
# Nestest
add_executable(nestest
    src/nestest_main.cpp
)

target_compile_features(nestest
    PRIVATE
        cxx_std_17
)

set_target_properties(nestest
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(nestest
    PRIVATE
        n_e_s::nes
        n_e_s::core
        n_e_s::warnings
        CONAN_PKG::fmt
)
