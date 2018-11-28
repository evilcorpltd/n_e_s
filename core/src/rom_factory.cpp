// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/rom_factory.h"

#include "rom.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

static_assert(sizeof(n_e_s::core::INesHeader) == 16);

namespace {

std::ifstream::pos_type filesize(const std::string &filename) {
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
    return file.tellg();
}

} // namespace

namespace n_e_s::core {

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

    printf("%s\n", h.nes);
    printf("%u\n", h.prg_rom_size);
    printf("%u\n", h.chr_rom_size);
    printf("%u\n", h.flags_6);
    printf("%u\n", h.flags_7);
    printf("%u\n", h.prg_ram_size);
    printf("%u\n", h.flags_9);
    printf("%u\n", h.flags_10);

    const uint32_t expected_rom_size = sizeof(INesHeader) +
            h.prg_rom_size * 16 * 1024 + h.chr_rom_size * 8 * 1024 +
            h.prg_ram_size * 8 * 1024;
    assert(bytes.size() == expected_rom_size);

    return new Rom(h);
}

} // namespace n_e_s::core
