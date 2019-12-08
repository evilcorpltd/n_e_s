project(n_e_s_warnings)

add_library(${PROJECT_NAME} INTERFACE)
add_library(n_e_s::warnings ALIAS ${PROJECT_NAME})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        set(CLANGCL ON)
    else()
        set(CLANG ON)
    endif()
endif()

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<OR:$<BOOL:${CLANG}>,$<CXX_COMPILER_ID:GNU>>:
        -fno-common;
        -pedantic-errors;
        -Wall;
        -Wdouble-promotion;
        -Wduplicated-cond;
        -Wduplicated-branches;
        -Werror;
        -Wextra;
        -Wformat-overflow;
        -Wformat-truncation;
        -Wformat=2;
        -Wlogical-op;
        -Wmissing-declarations;
        -Wnon-virtual-dtor;
        -Wnull-dereference;
        -Wold-style-cast;
        -Wshadow;
        -Wsign-compare;
        -Wundef;
        -Wuseless-cast;
    >
    $<$<BOOL:${CLANG}>:
        -Wmissing-variable-declarations;
        -Wno-unknown-warning-option;
    >
    $<$<OR:$<BOOL:${CLANGCL}>,$<CXX_COMPILER_ID:MSVC>>:
        /permissive-;
        /W4;
        /wd4244; # TODO(robinlinden): Enable this once IMmu::read_word is gone.
        /WX;
    >
)
