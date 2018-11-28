// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/rom_factory.h"

#include "rom.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>

#include <iostream>

static_assert(sizeof(n_e_s::core::INesHeader) == 16);

namespace {

std::ifstream::pos_type filesize(const std::string& filename)
{
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
    return file.tellg();
}

} // namespace

namespace n_e_s::core {

// IRom *RomFactory::fromFile(const std::filesystem::path& filepath) {
IRom *RomFactory::fromFile(const std::string& filepath) {
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

    return new Rom(h);
}

} // namespace n_e_s::core
