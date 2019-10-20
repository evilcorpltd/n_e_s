#include "nes/nes.h"

#include "core/cpu_factory.h"
#include "core/icpu.h"
#include "core/immu.h"
#include "core/ippu.h"
#include "core/irom.h"
#include "core/membank_factory.h"
#include "core/mmu_factory.h"
#include "core/ppu_factory.h"
#include "core/rom_factory.h"

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

class Nes::Impl {
public:
    Impl()
            : ppu_mmu(MmuFactory::create(
                      MemBankFactory::create_nes_ppu_mem_banks())),
              ppu(PpuFactory::create(&ppu_registers, ppu_mmu.get())),
              mmu(MmuFactory::create(
                      MemBankFactory::create_nes_mem_banks(ppu.get()))),
              cpu(CpuFactory::create_mos6502(&cpu_registers, mmu.get())) {}

    void execute() {
        if (cycle++ % 3 == 0) {
            cpu->execute();
        }

        ppu->execute();
    }

    void reset() {
        cpu->reset();
    }

    void load_rom(const std::string &filepath) {
        std::unique_ptr<IRom> rom{RomFactory::from_file(filepath)};

        MemBankList ppu_membanks{MemBankFactory::create_nes_ppu_mem_banks()};

        ppu_membanks.push_back(std::make_unique<MemBankReference>(rom.get()));

        ppu_mmu = MmuFactory::create(std::move(ppu_membanks));
        ppu = PpuFactory::create(&ppu_registers, ppu_mmu.get());

        MemBankList cpu_membanks{
                MemBankFactory::create_nes_mem_banks(ppu.get())};
        cpu_membanks.push_back(std::move(rom));

        mmu = MmuFactory::create(std::move(cpu_membanks));
        cpu = CpuFactory::create_mos6502(&cpu_registers, mmu.get());

        reset();
    }

    std::unique_ptr<IMmu> ppu_mmu;
    PpuRegisters ppu_registers{};
    std::unique_ptr<IPpu> ppu;

    std::unique_ptr<IMmu> mmu;

    CpuRegisters cpu_registers{};
    std::unique_ptr<ICpu> cpu;

    uint64_t cycle{};
};

Nes::Nes() : impl_(std::make_unique<Impl>()) {}

Nes::~Nes() = default;

void Nes::execute() {
    impl_->execute();
}

void Nes::reset() {
    impl_->reset();
}

void Nes::load_rom(const std::string &filepath) {
    impl_->load_rom(filepath);
}
n_e_s::core::ICpu &Nes::cpu() {
    return *impl_->cpu;
}
const n_e_s::core::ICpu &Nes::cpu() const {
    return *impl_->cpu;
}

n_e_s::core::IPpu &Nes::ppu() {
    return *impl_->ppu;
}
const n_e_s::core::IPpu &Nes::ppu() const {
    return *impl_->ppu;
}

n_e_s::core::IMmu &Nes::mmu() {
    return *impl_->mmu;
}
const n_e_s::core::IMmu &Nes::mmu() const {
    return *impl_->mmu;
}

n_e_s::core::IMmu &Nes::ppu_mmu() {
    return *impl_->ppu_mmu;
}
const n_e_s::core::IMmu &Nes::ppu_mmu() const {
    return *impl_->ppu_mmu;
}

n_e_s::core::CpuRegisters &Nes::cpu_registers() {
    return impl_->cpu_registers;
}
const n_e_s::core::CpuRegisters &Nes::cpu_registers() const {
    return impl_->cpu_registers;
}

n_e_s::core::PpuRegisters &Nes::ppu_registers() {
    return impl_->ppu_registers;
}
const n_e_s::core::PpuRegisters &Nes::ppu_registers() const {
    return impl_->ppu_registers;
}

uint64_t Nes::current_cycle() const {
    return impl_->cycle;
}

} // namespace n_e_s::nes
