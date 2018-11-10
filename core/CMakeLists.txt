project(n_e_s_core)

add_library(${PROJECT_NAME}
    src/cpu.h
    src/cpu.cpp
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Werror
        -Wextra
        -pedantic-errors
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_11
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

add_subdirectory(test)
