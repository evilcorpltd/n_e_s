#pragma once

#include "nes/core/iapu.h"

namespace n_e_s::core {

class Apu final : public IApu {
public:
    explicit Apu() = default;

    uint8_t read_byte(uint16_t addr) override;
    void write_byte(uint16_t addr, uint8_t byte) override;

    void execute() override;

    void set_sample_handler(
            const std::function<void(int8_t)> &sample_handler) override;

    void set_sample_handler_pulse_1(
            const std::function<void(int8_t)> &sample_handler) override;
    void set_sample_handler_pulse_2(
            const std::function<void(int8_t)> &sample_handler) override;
    void set_sample_handler_triangle(
            const std::function<void(int8_t)> &sample_handler) override;
    void set_sample_handler_noise(
            const std::function<void(int8_t)> &sample_handler) override;
    void set_sample_handler_pcm(
            const std::function<void(int8_t)> &sample_handler) override;

private:
    std::function<void(int8_t)> sample_handler_{[](int8_t) {}};
    std::function<void(int8_t)> sample_handler_pulse_1_{[](int8_t) {}};
    std::function<void(int8_t)> sample_handler_pulse_2_{[](int8_t) {}};
    std::function<void(int8_t)> sample_handler_triangle_{[](int8_t) {}};
    std::function<void(int8_t)> sample_handler_noise_{[](int8_t) {}};
    std::function<void(int8_t)> sample_handler_pcm_{[](int8_t) {}};

    // Dummys for stub
    uint16_t addr_{};
    uint8_t byte_{};
};

} // namespace n_e_s::core