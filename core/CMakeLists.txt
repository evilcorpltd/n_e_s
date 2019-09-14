project(n_e_s_core)

add_library(${PROJECT_NAME}
    include/core/cpu_factory.h
    include/core/membank_factory.h
    include/core/mmu_factory.h
    include/core/ppu_factory.h
    include/core/rom_factory.h
    include/core/icpu.h
    include/core/invalid_address.h
    include/core/imembank.h
    include/core/immu.h
    include/core/ippu.h
    include/core/irom.h
    src/cpu_factory.cpp
    src/invalid_address.cpp
    src/mapped_membank.h
    src/membank.h
    src/membank_base.h
    src/membank_factory.cpp
    src/mmu.cpp
    src/mmu.h
    src/mmu_factory.cpp
    src/mos6502.cpp
    src/mos6502.h
    src/opcode.cpp
    src/opcode.h
    src/pipeline.cpp
    src/pipeline.h
    src/ppu.cpp
    src/ppu.h
    src/ppu_factory.cpp
    src/rom_factory.cpp
    src/rom/nrom.cpp
    src/rom/nrom.h
)
add_library(n_e_s::core ALIAS ${PROJECT_NAME})

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
        n_e_s::warnings
)

add_subdirectory(test)
