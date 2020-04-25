include(FetchContent)

project(n_e_s_disassembler)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        567ed03f88d493b51e5efc24f9f4cad47dc5c76d # 6.2.0 with build fix for -Wshadow
)

FetchContent_MakeAvailable(fmt)

add_library(${PROJECT_NAME}
    include/nes/disassembler.h
    src/disassembler.cpp
)
add_library(n_e_s::disassembler ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        n_e_s::core
    PRIVATE
        fmt
        n_e_s::warnings
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

add_subdirectory(test)
