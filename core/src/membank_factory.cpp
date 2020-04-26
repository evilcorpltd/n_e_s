#include "nes/core/membank_factory.h"

#include <memory>

#include "mapped_membank.h"
#include "membank.h"
#include "nes/core/ippu.h"
#include "nes/core/irom.h"

namespace n_e_s::core {
namespace {

class CpuRomTranslator : public IMemBank {
public:
    explicit CpuRomTranslator(IRom *const rom) : rom_(rom) {}
    bool is_address_in_range(uint16_t addr) const override {
        return rom_->is_cpu_address_in_range(addr);
    }
    uint8_t read_byte(uint16_t addr) const override {
        return rom_->cpu_read_byte(addr);
    }
    void write_byte(uint16_t addr, uint8_t byte) override {
        rom_->cpu_write_byte(addr, byte);
    }

private:
    IRom *rom_;
};

class PpuRomTranslator : public IMemBank {
public:
    explicit PpuRomTranslator(IRom *const rom) : rom_(rom) {}
    bool is_address_in_range(uint16_t addr) const override {
        return rom_->is_ppu_address_in_range(addr);
    }
    uint8_t read_byte(uint16_t addr) const override {
        return rom_->ppu_read_byte(addr);
    }
    void write_byte(uint16_t addr, uint8_t byte) override {
        rom_->ppu_write_byte(addr, byte);
    }

private:
    IRom *rom_;
};

auto create_ppu_reader(IPpu *ppu) {
    return [ppu](uint16_t addr) { return ppu->read_byte(addr); };
}

auto create_ppu_writer(IPpu *ppu) {
    return [ppu](uint16_t addr, uint8_t byte) { ppu->write_byte(addr, byte); };
}

} // namespace

MemBankList MemBankFactory::create_nes_mem_banks(IPpu *ppu, IRom *rom) {
    MemBankList mem_banks;

    // Rom, make sure this is first so the mapper can decide if it wants to
    // handle the address or not.
    mem_banks.push_back(std::make_unique<CpuRomTranslator>(rom));

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

MemBankList MemBankFactory::create_nes_ppu_mem_banks(IRom *rom) {
    MemBankList mem_banks;

    // Rom, make sure this is first so the mapper can decide if it wants to
    // handle the address or not.
    mem_banks.push_back(std::make_unique<PpuRomTranslator>(rom));

    // Palettes
    mem_banks.push_back(std::make_unique<MemBank<0x3F00, 0x3FFF, 0x20>>());

    return mem_banks;
}

} // namespace n_e_s::core
