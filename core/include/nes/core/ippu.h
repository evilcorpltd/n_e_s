#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include "nes/core/pixel.h"
#include "ppu_registers.h"

namespace n_e_s::core {

class IPpu {
public:
    virtual ~IPpu() = default;

    virtual uint8_t read_byte(uint16_t addr) = 0;
    virtual void write_byte(uint16_t addr, uint8_t byte) = 0;

    virtual std::optional<Pixel> execute() = 0;

    virtual void set_nmi_handler(const std::function<void()> &nmi_handler) = 0;

    [[nodiscard]] virtual uint16_t scanline() const = 0;
    [[nodiscard]] virtual uint16_t cycle() const = 0;
};

} // namespace n_e_s::core
