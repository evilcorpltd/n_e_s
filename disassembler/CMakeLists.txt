project(n_e_s_disassembler)

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
