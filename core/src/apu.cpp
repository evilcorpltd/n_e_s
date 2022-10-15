#include "apu.h"

namespace n_e_s::core {

uint8_t Apu::read_byte(uint16_t addr) {
    addr_ = addr;
    return 0x00;
}
void Apu::write_byte(uint16_t addr, uint8_t byte) {
    addr_ = addr;
    byte_ = byte;
}

void Apu::execute() {
    // Dummy calls to sample handlers
    if (sample_handler_) {
        sample_handler_(0x01);
    }
    if (sample_handler_pulse_1_) {
        sample_handler_pulse_1_(0x02);
    }
    if (sample_handler_pulse_2_) {
        sample_handler_pulse_2_(0x03);
    }
    if (sample_handler_triangle_) {
        sample_handler_triangle_(0x04);
    }
    if (sample_handler_noise_) {
        sample_handler_noise_(0x05);
    }
    if (sample_handler_pcm_) {
        sample_handler_pcm_(0x06);
    }
}

void Apu::set_sample_handler(
        const std::function<void(int8_t)> &sample_handler) {
    sample_handler_ = sample_handler;
}

void Apu::set_sample_handler_pulse_1(
        const std::function<void(int8_t)> &sample_handler) {
    sample_handler_pulse_1_ = sample_handler;
}

void Apu::set_sample_handler_pulse_2(
        const std::function<void(int8_t)> &sample_handler) {
    sample_handler_pulse_2_ = sample_handler;
}

void Apu::set_sample_handler_triangle(
        const std::function<void(int8_t)> &sample_handler) {
    sample_handler_triangle_ = sample_handler;
}

void Apu::set_sample_handler_noise(
        const std::function<void(int8_t)> &sample_handler) {
    sample_handler_noise_ = sample_handler;
}

void Apu::set_sample_handler_pcm(
        const std::function<void(int8_t)> &sample_handler) {
    sample_handler_pcm_ = sample_handler;
}

} // namespace n_e_s::core