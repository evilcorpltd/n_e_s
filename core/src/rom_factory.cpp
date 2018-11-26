// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/rom_factory.h"

#include "rom.h"

namespace n_e_s::core {

IRom *RomFactory::fromFile(std::string path) {
    (void)path;
    return new Rom();
}

} // namespace n_e_s::core
