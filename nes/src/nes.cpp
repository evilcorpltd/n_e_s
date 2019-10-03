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
            : ppu_mmu_(MmuFactory::create(
                      MemBankFactory::create_nes_ppu_mem_banks())),
              ppu_(PpuFactory::create(&ppu_registers_, ppu_mmu_.get())),
              mmu_(MmuFactory::create(
                      MemBankFactory::create_nes_mem_banks(ppu_.get()))),
              cpu_(CpuFactory::create(&cpu_registers_, mmu_.get())) {}

    void execute() {
        if (cycle_++ % 3 == 0) {
            cpu_->execute();
        }

        ppu_->execute();
    }

    void reset() {
        cpu_->reset();
    }

    void load_rom(const std::string &filepath) {
        std::unique_ptr<IRom> rom{RomFactory::from_file(filepath)};

        MemBankList ppu_membanks{MemBankFactory::create_nes_ppu_mem_banks()};

        ppu_membanks.push_back(std::make_unique<MemBankReference>(rom.get()));

        ppu_mmu_ = MmuFactory::create(std::move(ppu_membanks));
        ppu_ = PpuFactory::create(&ppu_registers_, ppu_mmu_.get());

        MemBankList cpu_membanks{
                MemBankFactory::create_nes_mem_banks(ppu_.get())};
        cpu_membanks.push_back(std::move(rom));

        mmu_ = MmuFactory::create(std::move(cpu_membanks));
        cpu_ = CpuFactory::create(&cpu_registers_, mmu_.get());

        reset();
    }

private:
    std::unique_ptr<IMmu> ppu_mmu_;
    IPpu::Registers ppu_registers_{};
    std::unique_ptr<IPpu> ppu_;

    std::unique_ptr<IMmu> mmu_;

    ICpu::Registers cpu_registers_{};
    std::unique_ptr<ICpu> cpu_;

    uint64_t cycle_{};
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

} // namespace n_e_s::nes
