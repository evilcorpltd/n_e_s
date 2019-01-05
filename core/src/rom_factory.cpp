#include "core/rom_factory.h"

#include "rom/nrom.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

static_assert(sizeof(n_e_s::core::INesHeader) == 16);

namespace {

std::istream::pos_type streamsize(std::istream &stream) {
    stream.seekg(0, std::ios::end);
    std::istream::pos_type size = stream.tellg();
    stream.seekg(0, std::ios::beg);
    return size;
}

} // namespace

namespace n_e_s::core {

IRom *RomFactory::from_file(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        assert(false);
        exit(1); // Unable to open file.
    }

    return from_bytes(file);
}

IRom *RomFactory::from_bytes(std::istream &bytestream) {
    std::vector<uint8_t> bytes(streamsize(bytestream));
    if (bytes.size() < 16) {
        assert(false);
        exit(1); // File isn't big enough to contain a header.
    }

    if (!bytestream.read(reinterpret_cast<char *>(&bytes[0]), bytes.size())) {
        assert(false);
        exit(1); // Unable to get bytes.
    }

    INesHeader h;
    if (!std::equal(bytes.begin(), bytes.begin() + sizeof(h.nes), h.nes)) {
        assert(false);
        exit(1); // No valid iNes header.
    }

    // This is fine because the header is exactly 16 bytes with no padding.
    memcpy(&h, &bytes[0], sizeof(h));
    if (h.prg_ram_size == 0) {
        h.prg_ram_size = 1; // For compatibility reasons, 0 ram means 1 ram.
    }

    uint8_t mapper = (h.flags_6 & 0xF0) >> 4;
    mapper |= h.flags_7 & 0xF0;

    const uint32_t expected_rom_size = sizeof(INesHeader) +
                                       h.prg_rom_size * 16 * 1024 +
                                       h.chr_rom_size * 8 * 1024;
    if (bytes.size() != expected_rom_size) {
        assert(false);
        exit(1);
    }

    std::vector<uint8_t> prg_rom(bytes.begin() + 16,
            bytes.begin() + 16 + h.prg_rom_size * 16 * 1024);

    std::vector<uint8_t> chr_rom(
            bytes.begin() + 16 + h.prg_rom_size * 16 * 1024,
            bytes.begin() + 16 + h.prg_rom_size * 16 * 1024 +
                    h.chr_rom_size * 8 * 1024);

    if (mapper == 0) {
        return new Nrom(h, prg_rom, chr_rom);
    }

    std::stringstream err;
    err << "Unsupported mapper: " << +mapper;
    throw std::logic_error(err.str());
}

} // namespace n_e_s::core
