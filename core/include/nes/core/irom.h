#pragma once

#include "nes/core/imembank.h"
#include "nes/core/ines_header.h"

namespace n_e_s::core {

class IRom : public IMemBank {
public:
    explicit IRom(const INesHeader &h) : header_(h) {}

private:
    INesHeader header_;
};

} // namespace n_e_s::core
