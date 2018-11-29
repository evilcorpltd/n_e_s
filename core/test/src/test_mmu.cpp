// Copyright 2018 Robin Linden <dev@robinlinden.eu>

#include "core/mmu_factory.h"

#include <gtest/gtest.h>

using namespace n_e_s::core;

namespace {

class MmuTest : public ::testing::Test {
public:
    MmuTest() : mmu{MmuFactory::create()} {}

    std::unique_ptr<IMmu> mmu;
};



} // namespace
