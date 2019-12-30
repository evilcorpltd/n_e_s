#include "core/rom_factory.h"

#include "rom/nrom.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

static_assert(sizeof(n_e_s::core::INesHeader) == 16);

namespace {

size_t streamsize(std::istream &stream) {
    stream.seekg(0, std::ios::end);
    std::istream::pos_type size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    assert(size >= 0);
    assert(static_cast<uint64_t>(size) <= std::numeric_limits<size_t>::max());
    return static_cast<size_t>(size);
}

} // namespace

namespace n_e_s::core {

std::unique_ptr<IRom> RomFactory::from_file(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::invalid_argument("Unable to open file: " + filepath);
    }

    return from_bytes(file);
}

std::unique_ptr<IRom> RomFactory::from_bytes(std::istream &bytestream) {
    std::vector<uint8_t> bytes(streamsize(bytestream));
    if (bytes.size() < 16) {
        throw std::invalid_argument(
                "File isn't big enough to contain a header");
    }

    if (!bytestream.read(reinterpret_cast<char *>(&bytes[0]), bytes.size())) {
        throw std::invalid_argument("Unable to get bytes");
    }

    INesHeader h;
    if (!std::equal(bytes.begin(), bytes.begin() + sizeof(h.nes), h.nes)) {
        throw std::invalid_argument("No valid iNes header");
    }

    // This is fine because the header is exactly 16 bytes with no padding.
    memcpy(&h, &bytes[0], sizeof(h));
    if (h.prg_ram_size == 0) {
        h.prg_ram_size = 1; // For compatibility reasons, 0 ram means 1 ram.
    }

    uint8_t mapper = (h.flags_6 & 0xF0u) >> 4u;
    mapper |= h.flags_7 & 0xF0u;

    const uint32_t prg_rom_byte_count = h.prg_rom_size * 16 * 1024;
    const uint32_t chr_rom_byte_count = h.chr_rom_size * 8 * 1024;

    const uint32_t expected_rom_size =
            sizeof(INesHeader) + prg_rom_byte_count + chr_rom_byte_count;
    if (bytes.size() != expected_rom_size) {
        throw std::invalid_argument("Unexpected rom size");
    }

    std::vector<uint8_t> prg_rom(bytes.begin() + sizeof(INesHeader),
            bytes.begin() + sizeof(INesHeader) + prg_rom_byte_count);

    std::vector<uint8_t> chr_rom(
            bytes.begin() + sizeof(INesHeader) + prg_rom_byte_count,
            bytes.begin() + sizeof(INesHeader) + prg_rom_byte_count +
                    chr_rom_byte_count);

    if (mapper == 0) {
        return std::make_unique<Nrom>(h, prg_rom, chr_rom);
    }

    std::stringstream err;
    err << "Unsupported mapper: " << +mapper;
    throw std::logic_error(err.str());
}

} // namespace n_e_s::core
