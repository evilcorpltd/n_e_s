#pragma once

#include <cstdint>
#include <functional>

namespace n_e_s::core {

class IApu {
public:
    virtual ~IApu() = default;

    virtual uint8_t read_byte(uint16_t addr) = 0;
    virtual void write_byte(uint16_t addr, uint8_t byte) = 0;

    virtual void execute() = 0;

    virtual void set_sample_handler(
            const std::function<void(int8_t)> &audio_handler) = 0;

    // Set sample handlers for individual channels
    // (eg. for debugging, custom mixing etc)
    virtual void set_sample_handler_pulse_1(
            const std::function<void(int8_t)> &audio_handler) = 0;
    virtual void set_sample_handler_pulse_2(
            const std::function<void(int8_t)> &audio_handler) = 0;
    virtual void set_sample_handler_triangle(
            const std::function<void(int8_t)> &audio_handler) = 0;
    virtual void set_sample_handler_noise(
            const std::function<void(int8_t)> &audio_handler) = 0;
    virtual void set_sample_handler_pcm(
            const std::function<void(int8_t)> &audio_handler) = 0;
};

} // namespace n_e_s::core
