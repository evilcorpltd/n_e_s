#pragma once

#include <memory>

namespace n_e_s::nes {

class Nes {
public:
    Nes();
    ~Nes();

    void execute();
    void reset();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace n_e_s::nes
