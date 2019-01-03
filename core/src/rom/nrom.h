#pragma once

#include "core/irom.h"

#include <vector>

namespace n_e_s::core {

class Nrom : public IRom {
public:
    Nrom(const INesHeader &h,
            std::vector<uint8_t> prg_rom,
            std::vector<uint8_t> chr_rom);

    bool is_address_in_range(uint16_t addr) const override;
    uint8_t read_byte(uint16_t addr) const override;
    void write_byte(uint16_t addr, uint8_t byte) override;

private:
    std::vector<uint8_t> &translate_address(uint16_t addr);
    const std::vector<uint8_t> &translate_address(uint16_t addr) const;

    std::vector<uint8_t> prg_rom_; // const?
    std::vector<uint8_t> chr_rom_; // const?
    std::vector<uint8_t> prg_ram_;

    constexpr static uint16_t prg_ram_start_{0x6000};
    constexpr static uint16_t prg_ram_end_{0x7FFF};
    constexpr static uint16_t prg_rom_start_{0x8000};
};

} // namespace n_e_s::core
