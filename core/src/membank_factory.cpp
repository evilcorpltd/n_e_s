#include "core/membank_factory.h"

#include "core/ippu.h"

#include "mapped_membank.h"
#include "membank.h"

namespace n_e_s::core {
namespace {

auto create_ppu_reader(IPpu *ppu) {
    return [=](uint16_t addr) { return ppu->read_byte(addr); };
}

auto create_ppu_writer(IPpu *ppu) {
    return [=](uint16_t addr, uint8_t byte) { ppu->write_byte(addr, byte); };
}

} // namespace

MemBankList MemBankFactory::create_nes_mem_banks(IPpu *ppu) {
    MemBankList mem_banks;

    // Ram, repeats every 0x800 byte
    mem_banks.push_back(std::make_unique<MemBank<0x0000, 0x1FFF, 0x800>>());

    // Ppu, repeats every 0x8 byte
    mem_banks.push_back(std::make_unique<MappedMemBank<0x2000, 0x3FFF, 0x8>>(
            create_ppu_reader(ppu), create_ppu_writer(ppu)));

    // Io
    mem_banks.push_back(std::make_unique<MemBank<0x4000, 0x4017, 0x18>>());

    // Io dev
    mem_banks.push_back(std::make_unique<MemBank<0x4018, 0x401F, 0x8>>());

    return mem_banks;
}

MemBankList MemBankFactory::create_nes_ppu_mem_banks() {
    MemBankList mem_banks;

    // Empty for now
    return mem_banks;
}

} // namespace n_e_s::core
