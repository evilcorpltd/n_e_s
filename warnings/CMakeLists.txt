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
        -Werror;
        -Wextra;
        -Wnon-virtual-dtor;
        -Wshadow;
    >
    $<$<OR:$<BOOL:${CLANGCL}>,$<CXX_COMPILER_ID:MSVC>>:
        /permissive-;
        /WX;
    >
)
