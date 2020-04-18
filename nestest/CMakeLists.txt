include(FetchContent)

project(nestest)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        567ed03f88d493b51e5efc24f9f4cad47dc5c76d # 6.2.0 with build fix for -Wshadow
)

FetchContent_MakeAvailable(fmt)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::nes
        n_e_s::core
        n_e_s::warnings
        fmt
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
