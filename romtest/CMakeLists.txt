project(romtest)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::nes
        n_e_s::core
        n_e_s::disassembler
        n_e_s::warnings
        fmt
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
