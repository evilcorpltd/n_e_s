include(FetchContent)
include(GoogleTest)

project(test_n_e_s_core)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        e3f0319d89f4cbf32993de595d984183b1a9fc57 # 1.10 + build fix for clang on Windows
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME}
    src/cast_helpers.h
    src/cpu_test.h
    src/fake_mmu.h
    src/fake_ppu.h
    src/hexprinter.h
    src/icpu_helpers.cpp
    src/icpu_helpers.h
    src/main.cpp
    src/mock_membank.h
    src/mock_mmu.h
    src/mock_ppu.h
    src/opcode.h
    src/test_cpu.cpp
    src/test_cpu_absolute_instructions.cpp
    src/test_cpu_branch_instructions.cpp
    src/test_cpu_immediate_instructions.cpp
    src/test_cpu_move_instructions.cpp
    src/test_cpu_zeropage_indexed_instructions.cpp
    src/test_cpu_zeropage_instructions.cpp
    src/test_cpuintegration.cpp
    src/test_invalid_address.cpp
    src/test_mmu.cpp
    src/test_opcode.cpp
    src/test_ppu.cpp
    src/test_ppu_membank.cpp
    src/test_rom.cpp
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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        n_e_s::core
        n_e_s::warnings
        gmock
)

gtest_discover_tests(${PROJECT_NAME})
