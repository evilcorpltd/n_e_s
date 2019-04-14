#include "core/irom.h"
#include "core/rom_factory.h"

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
    header.flags_6 = mapper & 0x0F << 4;
    header.flags_7 = mapper & 0xF0;
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

TEST(RomFactory, cant_open_files_that_dont_exist) {
    EXPECT_THROW(RomFactory::from_file("this_file_hopefully_doesnt_exist"),
            std::invalid_argument);
}

TEST(RomFactory, doesnt_parse_streams_with_too_few_bytes) {
    std::string bytes(15, 0);
    std::stringstream ss(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);
}

TEST(RomFactory, doesnt_parse_bytes_without_a_nes_header) {
    std::string bytes(16, 0);
    std::stringstream ss(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);
}

TEST(RomFactory, fails_if_mapper_not_supported) {
    std::string bytes{ines_header_bytes(0xCD, 0, 0, 0)};
    std::stringstream ss(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::logic_error);
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

TEST(Nrom, has_the_correct_address_space) {
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom{RomFactory::from_bytes(ss)};
    EXPECT_TRUE(nrom->is_address_in_range(0x6000));
    EXPECT_TRUE(nrom->is_address_in_range(0xFFFF));
}

TEST(Nrom, creation_fails_with_bad_rom_sizes) {
    std::string bytes{nrom_bytes(0, 1)};
    std::stringstream ss(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 1);
    bytes.pop_back();
    ss = std::stringstream(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(3, 1);
    ss = std::stringstream(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 0);
    ss = std::stringstream(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);

    bytes = nrom_bytes(1, 2);
    ss = std::stringstream(bytes);
    EXPECT_THROW(RomFactory::from_bytes(ss), std::invalid_argument);
}

TEST(Nrom, write_and_read_byte) {
    std::string bytes{nrom_bytes(1, 1)};
    std::stringstream ss(bytes);
    std::unique_ptr<IRom> nrom = RomFactory::from_bytes(ss);

    nrom->write_byte(0x6000, 0x0F);
    EXPECT_EQ(0x0F, nrom->read_byte(0x6000));

    nrom->write_byte(0x8000, 0xAB);
    EXPECT_EQ(0xAB, nrom->read_byte(0x8000));
    EXPECT_EQ(0xAB, nrom->read_byte(0xC000));
    nrom->write_byte(0xBFFF, 0x10);
    EXPECT_EQ(0x10, nrom->read_byte(0xBFFF));
    EXPECT_EQ(0x10, nrom->read_byte(0xFFFF));
}

} // namespace
