#pragma once

#include <memory>
#include <optional>
#include <string>

#include "nes/core/pixel.h"

namespace n_e_s::core {
class IMos6502;
struct CpuRegisters;

class IPpu;
struct PpuRegisters;

class IApu;

class IMmu;

class IRom;

class INesController;

} // namespace n_e_s::core

namespace n_e_s::nes {

class Nes {
public:
    Nes();
    ~Nes();

    // Run at 263.25 / 11 Mhz for NTSC "realtime."
    std::optional<core::Pixel> execute();
    void reset();
    void load_rom(std::istream &bytestream);

    n_e_s::core::IMos6502 &cpu();
    const n_e_s::core::IMos6502 &cpu() const;

    n_e_s::core::IPpu &ppu();
    const n_e_s::core::IPpu &ppu() const;

    n_e_s::core::IMmu &mmu();
    const n_e_s::core::IMmu &mmu() const;

    n_e_s::core::IMmu &ppu_mmu();
    const n_e_s::core::IMmu &ppu_mmu() const;

    n_e_s::core::IApu &apu() const;

    n_e_s::core::CpuRegisters &cpu_registers();
    const n_e_s::core::CpuRegisters &cpu_registers() const;

    n_e_s::core::PpuRegisters &ppu_registers();
    const n_e_s::core::PpuRegisters &ppu_registers() const;

    n_e_s::core::INesController &controller1();
    const n_e_s::core::INesController &controller1() const;
    n_e_s::core::INesController &controller2();
    const n_e_s::core::INesController &controller2() const;

    uint64_t current_cycle() const;

private:
    std::unique_ptr<n_e_s::core::IMmu> ppu_mmu_;
    std::unique_ptr<n_e_s::core::PpuRegisters> ppu_registers_;
    std::unique_ptr<n_e_s::core::IPpu> ppu_;

    std::unique_ptr<n_e_s::core::IMmu> mmu_;

    std::unique_ptr<n_e_s::core::IApu> apu_;

    std::unique_ptr<n_e_s::core::CpuRegisters> cpu_registers_;
    std::unique_ptr<n_e_s::core::IMos6502> cpu_;

    std::unique_ptr<n_e_s::core::IRom> rom_;

    std::unique_ptr<n_e_s::core::INesController> controller1_;
    std::unique_ptr<n_e_s::core::INesController> controller2_;

    uint64_t cycle_{};
};

} // namespace n_e_s::nes
