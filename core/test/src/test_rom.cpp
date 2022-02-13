#include "ippu_helpers.h"
#include "nes/core/irom.h"
#include "nes/core/rom_factory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <sstream>

using namespace n_e_s::core;

namespace {

enum class Mapper {
    Nrom = 0,
    Mapper2 = 2,
};

std::string ines_header_bytes(const uint8_t mapper_id,
        const uint8_t prg_rom_size,
        const uint8_t chr_rom_size,
        const uint8_t prg_ram_size) {
    INesHeader header{};
    header.flags_6 =
            static_cast<uint8_t>(mapper_id & static_cast<uint8_t>(0x0Fu)) << 4u;
    header.flags_7 =
            static_cast<uint8_t>(mapper_id & static_cast<uint8_t>(0xF0u));
    header.prg_rom_size = prg_rom_size;
    header.chr_rom_size = chr_rom_size;
    header.prg_ram_size = prg_ram_size;

    std::string bytes;
    bytes.resize(sizeof(header));

    char *const raw = bytes.data();
    std::memcpy(raw, &header, sizeof(header));

    return bytes;
}

std::string nrom_bytes(const uint8_t prg_rom_size,
        const uint8_t chr_rom_size,
        const Mapper mapper) {
    std::string bytes{ines_header_bytes(
            static_cast<uint8_t>(mapper), prg_rom_size, chr_rom_size, 0)};

    bytes.append(prg_rom_size * 16 * 1024, 0);
    bytes.append(chr_rom_size * 8 * 1024, 0);

    return bytes;
}

void set_prg_rom_byte(const int prg_rom_banks,
        std::string *bytes,
        const uint32_t addr,
        const uint8_t data) {
    const uint32_t bytes_offset = addr + sizeof(INesHeader);
    const uint32_t prg_rom_size = prg_rom_banks * 16 * 1024;
    EXPECT_LT(addr, prg_rom_size);
    std::memcpy(&bytes->operator[](bytes_offset), &data, sizeof(data));
}

TEST(RomFactory, doesnt_parse_streams_with_too_few_bytes) {
    std::string bytes(15, 0);
    std::stringstream ss(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);
}

TEST(RomFactory, doesnt_parse_bytes_without_a_nes_header) {
    std::string bytes(16, 0);
    std::stringstream ss(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);
}

TEST(RomFactory, fails_if_mapper_not_supported) {
    std::string bytes{ines_header_bytes(0xCD, 0, 0, 0)};
    std::stringstream ss(bytes);
    EXPECT_THAT([&ss]() { auto tmp = RomFactory::from_bytes(ss); },
            testing::Throws<std::logic_error>(
                    testing::Property(&std::logic_error::what,
                            testing::HasSubstr("Unsupported mapper: 205"))));
}

TEST(Nrom, creation_works_with_correct_rom_sizes) {
    std::string bytes{nrom_bytes(1, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_NE(nullptr, nrom);

    bytes = nrom_bytes(2, 1, Mapper::Nrom);
    ss = std::stringstream(bytes);
    nrom = RomFactory::from_bytes(ss);
    EXPECT_NE(nullptr, nrom);
}

TEST(Nrom, has_the_correct_cpu_address_space) {
    std::string bytes{nrom_bytes(1, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_TRUE(nrom->is_cpu_address_in_range(0x6000));
    EXPECT_TRUE(nrom->is_cpu_address_in_range(0xFFFF));
}

TEST(Nrom, has_the_correct_ppu_address_space) {
    std::string bytes{nrom_bytes(1, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_TRUE(nrom->is_ppu_address_in_range(0x0000));
    EXPECT_TRUE(nrom->is_ppu_address_in_range(0x1FFF));
}

TEST(Nrom, creation_fails_with_bad_rom_sizes) {
    std::string bytes{nrom_bytes(0, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 1, Mapper::Nrom);
    bytes.pop_back();
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(3, 1, Mapper::Nrom);
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 2, Mapper::Nrom);
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);
}

////////////////////////////////////////////////////////////////
// Nrom tests
TEST(Nrom, write_and_read_byte_ppu_bus) {
    std::string bytes{nrom_bytes(1, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    nrom->ppu_write_byte(0x0100, 0x89);
    EXPECT_EQ(0x89, nrom->ppu_read_byte(0x0100));
}

TEST(Nrom, write_and_read_prg_ram) {
    std::string bytes{nrom_bytes(1, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    // $6000-$7FFF: prg ram
    nrom->cpu_write_byte(0x6000, 0x0F);
    EXPECT_EQ(0x0F, nrom->cpu_read_byte(0x6000));
    // Prg rom should not be affected.
    for (uint16_t addr = 0x8000; addr < 0xFFFF; ++addr) {
        EXPECT_EQ(0x00, nrom->cpu_read_byte(addr));
    }
}

TEST(Nrom, prg_rom_should_not_be_writable) {
    constexpr int kPrgRomBanks = 2;
    std::string bytes{nrom_bytes(kPrgRomBanks, 1, Mapper::Nrom)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    nrom->cpu_write_byte(0x8000, 0x11);
    EXPECT_EQ(0x00, nrom->cpu_read_byte(0x8000));
    nrom->cpu_write_byte(0xFFFF, 0x12);
    EXPECT_EQ(0x00, nrom->cpu_read_byte(0xFFFF));
}

TEST(Nrom, write_and_read_byte_cpu_bus_16k_prg_rom) {
    constexpr int kPrgRomBanks = 1;
    std::string bytes{nrom_bytes(kPrgRomBanks, 1, Mapper::Nrom)};
    set_prg_rom_byte(1, &bytes, 0x0000, 0xAB); // Mapped to 0x8000
    set_prg_rom_byte(1, &bytes, 0x3FFF, 0x10); // Mapped to 0xBFFF

    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    // 16 K prg rom: $C000-$FFFF should mirror $8000-$BFFF
    EXPECT_EQ(0xAB, nrom->cpu_read_byte(0x8000));
    EXPECT_EQ(0xAB, nrom->cpu_read_byte(0xC000));

    EXPECT_EQ(0x10, nrom->cpu_read_byte(0xBFFF));
    EXPECT_EQ(0x10, nrom->cpu_read_byte(0xFFFF));
}

TEST(Nrom, write_and_read_byte_cpu_bus_32k_prg_rom) {
    constexpr int kPrgRomBanks = 2;
    std::string bytes{nrom_bytes(kPrgRomBanks, 1, Mapper::Nrom)};
    set_prg_rom_byte(2, &bytes, 0x0000, 0xAB); // Mapped to 0x8000
    set_prg_rom_byte(2, &bytes, 0x3FFF, 0x10); // Mapped to 0xBFFF
    set_prg_rom_byte(2, &bytes, 0x4000, 0x42); // Mapped to 0xC000
    set_prg_rom_byte(2, &bytes, 0x7FFF, 0x78); // Mapped to 0xFFFF

    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    // 32 K prg rom:
    // CPU $8000-$BFFF: First 16 KB of ROM.
    // CPU $C000-$FFFF: Last 16 KB of ROM.
    EXPECT_EQ(0xAB, nrom->cpu_read_byte(0x8000));
    EXPECT_EQ(0x10, nrom->cpu_read_byte(0xBFFF));
    EXPECT_EQ(0x42, nrom->cpu_read_byte(0xC000));
    EXPECT_EQ(0x78, nrom->cpu_read_byte(0xFFFF));
}

////////////////////////////////////////////////////////////////
// Mapper 2 tests
TEST(Mapper2, write_and_read_byte_ppu_bus) {
    std::string bytes{nrom_bytes(1, 1, Mapper::Mapper2)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> rom = RomFactory::from_bytes(ss);

    rom->ppu_write_byte(0x0100, 0x89);
    EXPECT_EQ(0x89, rom->ppu_read_byte(0x0100));
}

TEST(Mapper2, write_should_not_modify_anything) {
    constexpr int kPrgRomBanks = 2;
    std::string bytes{nrom_bytes(kPrgRomBanks, 1, Mapper::Mapper2)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> rom = RomFactory::from_bytes(ss);

    rom->cpu_write_byte(0x9000, 0xF1);
    for (uint16_t addr = 0x8000; addr < 0xFFFF; ++addr) {
        EXPECT_EQ(0x00, rom->cpu_read_byte(addr));
    }
}

TEST(Mapper2, prg_rom_should_not_be_writable) {
    constexpr int kPrgRomBanks = 2;
    std::string bytes{nrom_bytes(kPrgRomBanks, 1, Mapper::Mapper2)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> rom = RomFactory::from_bytes(ss);

    rom->cpu_write_byte(0x8000, 0x11);
    EXPECT_EQ(0x00, rom->cpu_read_byte(0x8000));
    rom->cpu_write_byte(0xFFFF, 0x12);
    EXPECT_EQ(0x00, rom->cpu_read_byte(0xFFFF));
}

TEST(Mapper2, write_and_read_byte_cpu_bus) {
    constexpr int kPrgRomBanks = 8;
    std::string bytes{nrom_bytes(kPrgRomBanks, 1, Mapper::Mapper2)};
    // First byte in first bank
    set_prg_rom_byte(kPrgRomBanks, &bytes, 0u * 0x4000u, 0x01);
    // First byte in second bank
    set_prg_rom_byte(kPrgRomBanks, &bytes, 1u * 0x4000u, 0x02);
    // First byte in last bank
    set_prg_rom_byte(kPrgRomBanks, &bytes, 7u * 0x4000u, 0x07);
    // Second byte in last bank
    set_prg_rom_byte(kPrgRomBanks, &bytes, 7u * 0x4000u + 1u, 0xAB);

    std::stringstream ss(bytes);
    std::unique_ptr<IRom> rom = RomFactory::from_bytes(ss);

    // $8000-$BFFF: 16 KB switchable PRG ROM bank
    EXPECT_EQ(0x01, rom->cpu_read_byte(0x8000));

    // $C000-$FFFF: 16 KB PRG ROM bank, fixed to the last bank
    EXPECT_EQ(0x07, rom->cpu_read_byte(0xC000));
    EXPECT_EQ(0xAB, rom->cpu_read_byte(0xC001));

    // Write to rom to switch bank
    rom->cpu_write_byte(0x8000, 0x01);
    EXPECT_EQ(0x02, rom->cpu_read_byte(0x8000));

    // Last bank should not be affected
    EXPECT_EQ(0x07, rom->cpu_read_byte(0xC000));
    EXPECT_EQ(0xAB, rom->cpu_read_byte(0xC001));
}

} // namespace
