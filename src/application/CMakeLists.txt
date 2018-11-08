project(n_e_s_application)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s_core
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
