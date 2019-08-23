project(n_e_s_warnings)

add_library(${PROJECT_NAME} INTERFACE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        set(CLANGCL ON)
    else()
        set(CLANG ON)
    endif()
endif()

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<OR:$<BOOL:${CLANG}>,$<CXX_COMPILER_ID:GNU>>:
        -pedantic-errors;
        -Wall;
        -Wdouble-promotion;
        -Wduplicated-cond;
        -Werror;
        -Wextra;
        -Wformat=2;
        -Wlogical-op;
        -Wnon-virtual-dtor;
        -Wnull-dereference;
        -Wold-style-cast;
        -Wshadow;
    >
    $<$<OR:$<BOOL:${CLANGCL}>,$<CXX_COMPILER_ID:MSVC>>:
        /permissive-;
        /WX;
    >
)

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<BOOL:${CLANG}>:
      -Wno-unknown-warning-option;
    >
)
