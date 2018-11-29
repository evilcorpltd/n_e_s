// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/rom_factory.h"

#include "rom.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

static_assert(sizeof(n_e_s::core::INesHeader) == 16);

namespace {

std::ifstream::pos_type filesize(const std::string &filename) {
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
    if (!file) {
        return 0;
    }

    return file.tellg();
}

} // namespace

namespace n_e_s::core {

class Nrom : IRom {
public:
    Nrom(const INesHeader &header,
            std::vector<uint8_t> prg_ram,
            std::vector<uint8_t> prg_rom)
            : IRom(header),
              prg_ram_(std::move(prg_ram)),
              prg_rom_(std::move(prg_rom)) {
        assert(prg_ram_.size() == 2 * 1024 || prg_ram_.size() == 4 * 1024);
        assert(prg_rom_.size() == 16 * 1024 || prg_rom_.size() == 32 * 1024);
    }

    uint8_t read_byte(uint16_t addr) const override {
        assert(addr >= prg_ram_start_);

        const std::vector<uint8_t> &memory = translate_address(addr);
        addr -= prg_ram_start_;

        return memory[addr % memory.size()];
    }

    void write_byte(uint16_t addr, uint8_t byte) override {
        assert(addr >= prg_ram_start_);

        std::vector<uint8_t> &memory = translate_address(addr);
        addr -= prg_ram_start_;

        memory[addr % memory.size()] = byte;
    }

private:
    std::vector<uint8_t> &translate_address(uint16_t addr) {
        return const_cast<std::vector<uint8_t> &>(
                std::as_const(*this).translate_address(addr));
    }

    const std::vector<uint8_t> &translate_address(uint16_t addr) const {
        if (addr <= prg_ram_end_) {
            return prg_ram_;
        }

        return prg_rom_;
    }

    std::vector<uint8_t> prg_ram_;
    std::vector<uint8_t> prg_rom_; // TODO: const?

    constexpr static uint16_t prg_ram_start_{0x6000};
    constexpr static uint16_t prg_ram_end_{0x7FFF};
    constexpr static uint16_t prg_rom_start_{0x8000};
};

IRom *RomFactory::fromFile(const std::string &filepath) {
    std::vector<uint8_t> bytes(filesize(filepath));

    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        assert(false);
        exit(1); // Unable to open file.
    }

    if (bytes.size() < 16) {
        assert(false);
        exit(1); // File isn't big enough to contain a header.
    }

    if (!file.read(reinterpret_cast<char *>(&bytes[0]), bytes.size())) {
        assert(false);
        exit(1); // Unable to get file bytes.
    }

    INesHeader h;
    if (!std::equal(bytes.begin(), bytes.begin() + sizeof(h.nes), h.nes)) {
        assert(false);
        exit(1); // No valid iNes header.
    }

    memcpy(&h, &bytes[0], sizeof(h));

    // Not null terminated, so we'll get some trash, but that's fine.
    printf("%s\n", h.nes);
    printf("%u\n", h.prg_rom_size);
    printf("%u\n", h.chr_rom_size);
    printf("%u\n", h.flags_6);
    printf("%u\n", h.flags_7);
    printf("%u\n", h.prg_ram_size);
    printf("%u\n", h.flags_9);
    printf("%u\n", h.flags_10);

    uint8_t mapper = (h.flags_6 & 0xF0) >> 4;
    mapper |= h.flags_7 & 0xF0;

    const uint32_t expected_rom_size =
            sizeof(INesHeader) + h.prg_rom_size * 16 * 1024 +
            h.chr_rom_size * 8 * 1024 + h.prg_ram_size * 8 * 1024;
    assert(bytes.size() == expected_rom_size);

    if (mapper == 0) {
        return nullptr; // TODO: Mapper 0.
    }

    std::stringstream err;
    err << "Unsupported mapper: " << +mapper;
    throw std::logic_error(err.str());
}

} // namespace n_e_s::core
