cmake_minimum_required(VERSION 3.10)

project(n_e_s)

add_executable(${PROJECT_NAME}
    cpu.h
    cpu.cpp
    main.cpp
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
