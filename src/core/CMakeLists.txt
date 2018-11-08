project(n_e_s_core)

add_library(${PROJECT_NAME}
    cpu.h
    cpu.cpp
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

message(INFO ${CMAKE_SOURCE_DIR})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)
