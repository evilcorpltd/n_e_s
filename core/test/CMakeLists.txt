include(GoogleTest)

project(test_n_e_s_core)

add_executable(${PROJECT_NAME}
    src/cast_helpers.h
    src/cpu_test.h
    src/icpu_helpers.cpp
    src/icpu_helpers.h
    src/ippu_helpers.cpp
    src/ippu_helpers.h
    src/main.cpp
    src/opcode.h
    src/test_cpu.cpp
    src/test_cpu_absolute_indexed_instructions.cpp
    src/test_cpu_absolute_instructions.cpp
    src/test_cpu_branch_instructions.cpp
    src/test_cpu_immediate_instructions.cpp
    src/test_cpu_indexed_indirect_instructions.cpp
    src/test_cpu_indirect_indexed_instructions.cpp
    src/test_cpu_move_instructions.cpp
    src/test_cpu_nop_absolute_x_instructions.cpp
    src/test_cpu_nop_implied_instructions.cpp
    src/test_cpu_nop_zeropage_x_instructions.cpp
    src/test_cpu_zeropage_indexed_instructions.cpp
    src/test_cpu_zeropage_instructions.cpp
    src/test_cpuintegration.cpp
    src/test_ines_header.cpp
    src/test_invalid_address.cpp
    src/test_mmu.cpp
    src/test_nes_controller.cpp
    src/test_opcode.cpp
    src/test_ppu.cpp
    src/test_ppu_membank.cpp
    src/test_rom.cpp
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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::core
        n_e_s::core::test_utils
        n_e_s::warnings
        gmock
        fmt
)

gtest_discover_tests(${PROJECT_NAME})
