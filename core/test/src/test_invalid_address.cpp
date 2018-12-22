#include "core/invalid_address.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

TEST(InvalidAddress, throw_invalid_address) {
    std::string expected("Invalid address [0xABCD]");

    try {
        throw InvalidAddress(0xABCD);
    } catch (const InvalidAddress &e) {
        EXPECT_STREQ(e.what(), expected.c_str());
    }
}

} // namespace