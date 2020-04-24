#pragma once

#include <cstdint>
#include <exception>
#include <string>

namespace n_e_s::core {

class InvalidAddress : public std::exception {
public:
    explicit InvalidAddress(uint16_t addr);

    const char *what() const noexcept override;

private:
    std::string message_;
};

} // namespace n_e_s::core
