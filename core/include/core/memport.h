// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#pragma once

#include <cstdint>
#include <functional>

namespace n_e_s::core {

class MemPort {
public:
    using Reader = std::function<uint8_t(uint16_t)>;
    using Writer = std::function<void(uint16_t, uint8_t)>;

    MemPort(Reader reader, Writer writer);

    uint8_t read(uint16_t addr) const;

    void write(uint16_t addr, uint8_t byte);

private:
    Reader reader_;
    Writer writer_;
};

} // namespace n_e_s::core