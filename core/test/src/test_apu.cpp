#include "nes/core/apu_factory.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class ApuTest : public ::testing::Test {
public:
    ApuTest() : apu(ApuFactory::create()) {}

    void step_execution(uint32_t cycles) {
        for (uint32_t i = 0; i < cycles; ++i) {
            apu->execute();
        }
    }

    std::unique_ptr<IApu> apu;
};

// Placeholder for future tests
// For now only dummy test...
TEST_F(ApuTest, dummy_test) {
    EXPECT_EQ(0x00, apu->read_byte(0x1234));
    apu->write_byte(0x0000, 0x00);
    int8_t sample{};
    int8_t sample_pulse_1{};
    int8_t sample_pulse_2{};
    int8_t sample_triangle{};
    int8_t sample_noise{};
    int8_t sample_pcm{};
    apu->execute();
    apu->set_sample_handler([&sample](int8_t s) { sample = s; });
    apu->set_sample_handler_pulse_1(
            [&sample_pulse_1](int8_t s) { sample_pulse_1 = s; });
    apu->set_sample_handler_pulse_2(
            [&sample_pulse_2](int8_t s) { sample_pulse_2 = s; });
    apu->set_sample_handler_triangle(
            [&sample_triangle](int8_t s) { sample_triangle = s; });
    apu->set_sample_handler_noise(
            [&sample_noise](int8_t s) { sample_noise = s; });
    apu->set_sample_handler_pcm([&sample_pcm](int8_t s) { sample_pcm = s; });
    apu->execute();
    EXPECT_EQ(sample, 0x01);
    EXPECT_EQ(sample_pulse_1, 0x02);
    EXPECT_EQ(sample_pulse_2, 0x03);
    EXPECT_EQ(sample_triangle, 0x04);
    EXPECT_EQ(sample_noise, 0x05);
    EXPECT_EQ(sample_pcm, 0x06);
}

} // namespace