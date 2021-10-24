#include "nes/core/irom.h"
#include "nes/core/rom_factory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <sstream>

using namespace n_e_s::core;

namespace {

std::string ines_header_bytes(const uint8_t mapper,
        const uint8_t prg_rom_size,
        const uint8_t chr_rom_size,
        const uint8_t prg_ram_size) {
    INesHeader header{};
    header.flags_6 = static_cast<uint8_t>(mapper & static_cast<uint8_t>(0x0Fu))
                     << 4u;
    header.flags_7 = static_cast<uint8_t>(mapper & static_cast<uint8_t>(0xF0u));
    header.prg_rom_size = prg_rom_size;
    header.chr_rom_size = chr_rom_size;
    header.prg_ram_size = prg_ram_size;

    std::string bytes;
    bytes.resize(sizeof(header));

    char *const raw = bytes.data();
    std::memcpy(raw, &header, sizeof(header));

    return bytes;
}

std::string nrom_bytes(const uint8_t prg_rom_size, const uint8_t chr_rom_size) {
    std::string bytes{ines_header_bytes(0, prg_rom_size, chr_rom_size, 0)};

    bytes.append(prg_rom_size * 16 * 1024, 0);
    bytes.append(chr_rom_size * 8 * 1024, 0);

    return bytes;
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
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_NE(nullptr, nrom);

    bytes = nrom_bytes(2, 1);
    ss = std::stringstream(bytes);
    nrom = RomFactory::from_bytes(ss);
    EXPECT_NE(nullptr, nrom);
}

TEST(Nrom, has_the_correct_cpu_address_space) {
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_TRUE(nrom->is_cpu_address_in_range(0x6000));
    EXPECT_TRUE(nrom->is_cpu_address_in_range(0xFFFF));
}

TEST(Nrom, has_the_correct_ppu_address_space) {
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_TRUE(nrom->is_ppu_address_in_range(0x0000));
    EXPECT_TRUE(nrom->is_ppu_address_in_range(0x1FFF));
}

TEST(Nrom, creation_fails_with_bad_rom_sizes) {
    std::string bytes{nrom_bytes(0, 1)};
    std::stringstream ss(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 1);
    bytes.pop_back();
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(3, 1);
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 0);
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 2);
    ss = std::stringstream(bytes);
    EXPECT_THROW(auto tmp = RomFactory::from_bytes(ss), std::invalid_argument);
}

TEST(Nrom, write_and_read_byte_ppu_bus) {
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    nrom->ppu_write_byte(0x0100, 0x89);
    EXPECT_EQ(0x89, nrom->ppu_read_byte(0x0100));
}

TEST(Nrom, write_and_read_byte_cpu_bus) {
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    nrom->cpu_write_byte(0x6000, 0x0F);
    EXPECT_EQ(0x0F, nrom->cpu_read_byte(0x6000));

    nrom->cpu_write_byte(0x8000, 0xAB);
    EXPECT_EQ(0xAB, nrom->cpu_read_byte(0x8000));
    EXPECT_EQ(0xAB, nrom->cpu_read_byte(0xC000));
    nrom->cpu_write_byte(0xBFFF, 0x10);
    EXPECT_EQ(0x10, nrom->cpu_read_byte(0xBFFF));
    EXPECT_EQ(0x10, nrom->cpu_read_byte(0xFFFF));
}

} // namespace
