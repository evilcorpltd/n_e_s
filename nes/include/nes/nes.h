#pragma once

#include <memory>
#include <string>

namespace n_e_s::nes {

class Nes {
public:
    Nes();
    ~Nes();

    void execute();
    void reset();
    void load_rom(const std::string &filepath);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace n_e_s::nes
