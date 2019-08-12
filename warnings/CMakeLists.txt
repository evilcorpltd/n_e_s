project(n_e_s_warnings)

add_library(${PROJECT_NAME} INTERFACE)

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -pedantic-errors;
        -Wall;
        -Werror;
        -Wextra;
        -Wnon-virtual-dtor;
        -Wshadow;
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /permissive-;
        /WX;
    >
)
