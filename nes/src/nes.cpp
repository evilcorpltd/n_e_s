#include "nes/nes.h"

#include "core/cpu_factory.h"
#include "core/icpu.h"
#include "core/immu.h"
#include "core/ippu.h"
#include "core/membank_factory.h"
#include "core/mmu_factory.h"
#include "core/ppu_factory.h"

using namespace n_e_s::core;

namespace n_e_s::nes {

class Nes::Impl {
public:
    Impl()
            : ppu_(PpuFactory::create(&ppu_registers_)),
              mmu_(MmuFactory::create(
                      MemBankFactory::create_nes_mem_banks(ppu_.get()))),
              cpu_(CpuFactory::create(&registers_, mmu_.get())) {}

    void execute() {
        if (cycle_++ % 3 == 0) {
            cpu_->execute();
        }

        ppu_->execute();
    }

    void reset() {
        cpu_->reset();
    }

private:
    PpuRegisters ppu_registers_{};
    std::unique_ptr<IPpu> ppu_;

    std::unique_ptr<IMmu> mmu_;

    Registers registers_{};
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

} // namespace n_e_s::nes
