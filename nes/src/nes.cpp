#include "nes/nes.h"

#include "nes/core/cpu_factory.h"
#include "nes/core/immu.h"
#include "nes/core/imos6502.h"
#include "nes/core/ippu.h"
#include "nes/core/irom.h"
#include "nes/core/membank_factory.h"
#include "nes/core/mmu_factory.h"
#include "nes/core/ppu_factory.h"
#include "nes/core/rom_factory.h"

using namespace n_e_s::core;

namespace n_e_s::nes {
namespace {

class MemBankReference : public IMemBank {
public:
    explicit MemBankReference(IMemBank *const membank) : membank_(membank) {}

    bool is_address_in_range(uint16_t addr) const override {
        return membank_->is_address_in_range(addr);
    }

    uint8_t read_byte(uint16_t addr) const override {
        return membank_->read_byte(addr);
    }
    void write_byte(uint16_t addr, uint8_t byte) override {
        membank_->write_byte(addr, byte);
    }

private:
    IMemBank *membank_;
};

} // namespace

Nes::Nes()
        : ppu_mmu_(MmuFactory::create_empty()),
          ppu_registers_(std::make_unique<n_e_s::core::PpuRegisters>()),
          ppu_(PpuFactory::create(ppu_registers_.get(), ppu_mmu_.get())),
          mmu_(MmuFactory::create_empty()),
          cpu_registers_(std::make_unique<n_e_s::core::CpuRegisters>()),
          cpu_(CpuFactory::create_mos6502(cpu_registers_.get(),
                  mmu_.get(),
                  ppu_.get())) {
    // P should be set to 0x34 according to the information here:
    // https://wiki.nesdev.com/w/index.php/CPU_power_up_state
    // However, nestest sets p to 0x24 instead. We do that for now as well.
    cpu_registers_->p = I_FLAG | FLAG_5;
    cpu_registers_->a = cpu_registers_->x = cpu_registers_->y = 0x00;
    cpu_registers_->sp = 0xFD;
}

Nes::~Nes() = default;

// https://wiki.nesdev.com/w/index.php/Cycle_reference_chart#Clock_rates
void Nes::execute() {
    if (cycle_++ % 12 == 0) {
        cpu_->execute();
    }

    if (cycle_ % 4 == 0) {
        ppu_->execute();
    }

    // The APU runs at master clock % 24. (every other CPU tick)
}

void Nes::reset() {
    cpu_->reset();
}

void Nes::load_rom(const std::string &filepath) {
    rom_ = RomFactory::from_file(filepath);

    MemBankList ppu_membanks{
            MemBankFactory::create_nes_ppu_mem_banks(rom_.get())};
    ppu_mmu_->set_mem_banks(std::move(ppu_membanks));

    MemBankList cpu_membanks{
            MemBankFactory::create_nes_mem_banks(ppu_.get(), rom_.get())};
    mmu_->set_mem_banks(std::move(cpu_membanks));

    reset();
}

n_e_s::core::IMos6502 &Nes::cpu() {
    return *cpu_;
}
const n_e_s::core::IMos6502 &Nes::cpu() const {
    return *cpu_;
}

n_e_s::core::IPpu &Nes::ppu() {
    return *ppu_;
}
const n_e_s::core::IPpu &Nes::ppu() const {
    return *ppu_;
}

n_e_s::core::IMmu &Nes::mmu() {
    return *mmu_;
}
const n_e_s::core::IMmu &Nes::mmu() const {
    return *mmu_;
}

n_e_s::core::IMmu &Nes::ppu_mmu() {
    return *ppu_mmu_;
}
const n_e_s::core::IMmu &Nes::ppu_mmu() const {
    return *ppu_mmu_;
}

n_e_s::core::CpuRegisters &Nes::cpu_registers() {
    return *cpu_registers_;
}
const n_e_s::core::CpuRegisters &Nes::cpu_registers() const {
    return *cpu_registers_;
}

n_e_s::core::PpuRegisters &Nes::ppu_registers() {
    return *ppu_registers_;
}
const n_e_s::core::PpuRegisters &Nes::ppu_registers() const {
    return *ppu_registers_;
}

uint64_t Nes::current_cycle() const {
    return cycle_;
}

} // namespace n_e_s::nes
