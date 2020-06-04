#pragma once

#include <memory>
#include <string>

namespace n_e_s::core {
class IMos6502;
struct CpuRegisters;

class IPpu;
struct PpuRegisters;

class IMmu;

class IRom;
} // namespace n_e_s::core

namespace n_e_s::nes {

class Nes {
public:
    Nes();
    ~Nes();

    void execute();
    void reset();
    void load_rom(const std::string &filepath);

    n_e_s::core::IMos6502 &cpu();
    const n_e_s::core::IMos6502 &cpu() const;

    n_e_s::core::IPpu &ppu();
    const n_e_s::core::IPpu &ppu() const;

    n_e_s::core::IMmu &mmu();
    const n_e_s::core::IMmu &mmu() const;

    n_e_s::core::IMmu &ppu_mmu();
    const n_e_s::core::IMmu &ppu_mmu() const;

    n_e_s::core::CpuRegisters &cpu_registers();
    const n_e_s::core::CpuRegisters &cpu_registers() const;

    n_e_s::core::PpuRegisters &ppu_registers();
    const n_e_s::core::PpuRegisters &ppu_registers() const;

    uint64_t current_cycle() const;

private:
    std::unique_ptr<n_e_s::core::IMmu> ppu_mmu_;
    std::unique_ptr<n_e_s::core::PpuRegisters> ppu_registers_;
    std::unique_ptr<n_e_s::core::IPpu> ppu_;

    std::unique_ptr<n_e_s::core::IMmu> mmu_;

    std::unique_ptr<n_e_s::core::CpuRegisters> cpu_registers_;
    std::unique_ptr<n_e_s::core::IMos6502> cpu_;

    std::unique_ptr<n_e_s::core::IRom> rom_;

    uint64_t cycle_{};
};

} // namespace n_e_s::nes
