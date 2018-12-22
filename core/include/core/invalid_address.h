#pragma once

#include <stdexcept>
#include <string>

namespace n_e_s::core {

class InvalidAddress : public std::exception {
public:
    InvalidAddress(uint16_t addr);

    const char *what() const noexcept;

private:
    std::string message_;
};

} // namespace n_e_s::core
