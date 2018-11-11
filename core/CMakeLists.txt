project(n_e_s_core)

add_library(${PROJECT_NAME}
    include/core/icpu.h
    include/core/immu.h
    src/cpu.h
    src/cpu.cpp
    src/mmu.h
    src/mmu.cpp
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
        cxx_std_17
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        src
        include
)

add_subdirectory(test)
