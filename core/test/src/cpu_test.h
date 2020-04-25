#pragma once

#include "fake_ppu.h"
#include "mock_mmu.h"

#include "nes/core/cpu_factory.h"
#include "nes/core/imos6502.h"

#include <gtest/gtest.h>

#include <memory>

namespace n_e_s::core::test {

class CpuTest : public ::testing::Test {
public:
    CpuTest()
            : registers(),
              mmu(),
              ppu(),
              cpu{CpuFactory::create_mos6502(&registers, &mmu, &ppu)},
              expected() {
        registers.sp = expected.sp = 0xFF;
    }

    void stage_instruction(uint8_t instruction) {
        expected.pc += 1;
        EXPECT_CALL(mmu, read_byte(registers.pc))
                .WillOnce(testing::Return(instruction));
    }

    void step_execution(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            cpu->execute();
        }
    }

    CpuRegisters registers;
    testing::NiceMock<MockMmu> mmu;
    FakePpu ppu;
    std::unique_ptr<IMos6502> cpu;

    CpuRegisters expected;

    testing::InSequence in_sequence{};
};

} // namespace n_e_s::core::test
