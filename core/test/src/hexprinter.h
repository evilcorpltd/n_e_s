#pragma once

#include <climits>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace n_e_s::core::test {

constexpr int kHexDigitBits = 4;

template <typename T>
struct IsChar : std::integral_constant<bool,
                        std::is_same<T, char>::value ||
                                std::is_same<T, signed char>::value ||
                                std::is_same<T, unsigned char>::value> {};

// Adapted from https://stackoverflow.com/a/35963334
template <typename T>
std::string hex_out_s(T val) {
    std::stringstream sformatter;
    sformatter << std::hex << std::internal << "0x" << std::setfill('0')
               << std::setw(sizeof(T) * CHAR_BIT / kHexDigitBits)
               << (IsChar<T>::value ? static_cast<int>(val) : val);

    return sformatter.str();
}
} // namespace n_e_s::core::test
