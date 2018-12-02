project(n_e_s_core)

add_library(${PROJECT_NAME}
    include/core/cpu_factory.h
    include/core/mmu_factory.h
    include/core/ppu_factory.h
    include/core/icpu.h
    include/core/imembank.h
    include/core/immu.h
    include/core/ippu.h
    include/core/membank_factory.h
    src/cpu_factory.cpp
    src/membank.h
    src/membank_factory.cpp
    src/mmu.h
    src/mmu.cpp
    src/mmu_factory.cpp
    src/mos6502.h
    src/mos6502.cpp
    src/ppu.h
    src/ppu.cpp
    src/ppu_factory.cpp
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
