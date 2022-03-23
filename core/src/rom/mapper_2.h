#pragma once

#include "nes/core/irom.h"

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace n_e_s::core {

class Mapper2 : public IRom {
public:
    Mapper2(const INesHeader &h,
            std::vector<uint8_t> prg_rom,
            std::vector<uint8_t> chr_mem);

    [[nodiscard]] bool is_cpu_address_in_range(uint16_t addr) const override;
    uint8_t cpu_read_byte(uint16_t addr) const override;
    void cpu_write_byte(uint16_t addr, uint8_t byte) override;

    [[nodiscard]] bool is_ppu_address_in_range(uint16_t addr) const override;
    uint8_t ppu_read_byte(uint16_t addr) const override;
    void ppu_write_byte(uint16_t addr, uint8_t byte) override;

private:
    std::pair<int, uint16_t> translate_nametable_addr(uint16_t addr,
            Mirroring m) const;

    uint8_t select_bank_low_{0u};
    uint8_t select_bank_hi_{0u};
    std::vector<uint8_t> prg_rom_; // const?
    std::vector<uint8_t> chr_mem_;

    std::array<std::array<uint8_t, 0x0400>, 2> nametables_;

    constexpr static uint16_t kChrEnd{0x1FFF};
    constexpr static uint16_t kNametableStart{0x2000};
    constexpr static uint16_t kNametableEnd{0x3EFF};

    constexpr static uint16_t kSwitchablePrgRomStart{0x8000};
    constexpr static uint16_t kSwitchablePrgRomEnd{0xBFFF};
    constexpr static uint16_t kLastBankPrgRomStart{0xC000};
    constexpr static uint16_t kLastBankPrgRomEnd{0xFFFF};
};

} // namespace n_e_s::core
