#include "core/invalid_address.h"

#include <iomanip>
#include <sstream>

namespace n_e_s::core {

InvalidAddress::InvalidAddress(uint16_t addr) : message_() {
    std::stringstream ss;
    ss << "Invalid address [0x";
    ss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex
       << addr << "]";

    message_ = ss.str();
}

const char *InvalidAddress::what() const throw() {
    return message_.c_str();
}

} // namespace n_e_s::core
