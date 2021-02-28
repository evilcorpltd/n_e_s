project(n_e_s_core)

add_library(${PROJECT_NAME}
    include/nes/core/cpu_factory.h
    include/nes/core/icpu.h
    include/nes/core/imembank.h
    include/nes/core/immu.h
    include/nes/core/ines_controller.h
    include/nes/core/imos6502.h
    include/nes/core/ines_header.h
    include/nes/core/invalid_address.h
    include/nes/core/ippu.h
    include/nes/core/irom.h
    include/nes/core/membank_factory.h
    include/nes/core/mmu_factory.h
    include/nes/core/nes_controller_factory.h
    include/nes/core/opcode.h
    include/nes/core/ppu_factory.h
    include/nes/core/rom_factory.h
    src/cpu_factory.cpp
    src/invalid_address.cpp
    src/mapped_membank.h
    src/membank.h
    src/membank_base.h
    src/membank_controller_io.h
    src/membank_factory.cpp
    src/mmu.cpp
    src/mmu.h
    src/mmu_factory.cpp
    src/nes_controller.h
    src/nes_controller_factory.cpp
    src/mos6502.cpp
    src/mos6502.h
    src/opcode.cpp
    src/pipeline.cpp
    src/pipeline.h
    src/ppu.cpp
    src/ppu.h
    src/ppu_factory.cpp
    src/rom/nrom.cpp
    src/rom/nrom.h
    src/rom_factory.cpp
)
add_library(n_e_s::core ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    PRIVATE
        src
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::warnings
)

add_subdirectory(test_utils)
add_subdirectory(test)
