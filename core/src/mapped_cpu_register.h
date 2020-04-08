#pragma once

#include "core/imembank.h"
#include "core/ippu.h"

namespace n_e_s::core {

class MappedCpuRegister : public IMemBank {
public:
    MappedCpuRegister(IPpu *ppu, IMemBank *cpu_ram);

    bool is_address_in_range(uint16_t addr) const override;

    uint8_t read_byte(uint16_t addr) const override;
    void write_byte(uint16_t addr, uint8_t byte) override;

private:
    void do_dma(uint8_t byte);

    IPpu *ppu_;
    IMemBank *cpu_ram_;
};

} // namespace n_e_s::core
