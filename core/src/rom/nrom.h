#pragma once

#include "nes/core/irom.h"

#include <array>
#include <vector>

namespace n_e_s::core {

class Nrom : public IRom {
public:
    Nrom(const INesHeader &h,
            std::vector<uint8_t> prg_rom,
            std::vector<uint8_t> chr_rom);

    [[nodiscard]] bool is_cpu_address_in_range(uint16_t addr) const override;
    uint8_t cpu_read_byte(uint16_t addr) const override;
    void cpu_write_byte(uint16_t addr, uint8_t byte) override;

    [[nodiscard]] bool is_ppu_address_in_range(uint16_t addr) const override;
    uint8_t ppu_read_byte(uint16_t addr) const override;
    void ppu_write_byte(uint16_t addr, uint8_t byte) override;

private:
    std::pair<int, uint16_t> translate_nametable_addr(uint16_t addr,
            Mirroring m) const;

    std::vector<uint8_t> prg_rom_; // const?
    std::vector<uint8_t> chr_rom_; // const?
    std::vector<uint8_t> prg_ram_;

    std::array<std::array<uint8_t, 0x0400>, 2> nametables_;

    constexpr static uint16_t kChrEnd{0x1FFF};
    constexpr static uint16_t kNametableStart{0x2000};
    constexpr static uint16_t kNametableEnd{0x3EFF};

    constexpr static uint16_t kPrgRamStart{0x6000};
    constexpr static uint16_t kPrgRamEnd{0x7FFF};
    constexpr static uint16_t kPrgRomStart{0x8000};
};

} // namespace n_e_s::core
