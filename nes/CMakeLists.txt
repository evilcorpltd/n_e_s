project(n_e_s_nes)

add_library(${PROJECT_NAME}
    include/nes/nes.h
    src/nes.cpp
)
add_library(n_e_s::nes ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    PRIVATE
        src
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        n_e_s::core
    PRIVATE
        n_e_s::warnings
)

add_subdirectory(test)
