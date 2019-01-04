project(n_e_s_nes)

add_library(${PROJECT_NAME}
    include/nes/nes.h
    src/nes.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME}
        PRIVATE
           -Wall
           -Werror
           -Wextra
           -Wswitch-enum
           -Wshadow
           -Wnon-virtual-dtor
           -pedantic-errors
    )
endif()

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    PRIVATE
        src
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s_core
)

add_subdirectory(test)
