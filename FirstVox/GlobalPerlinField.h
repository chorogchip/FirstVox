#pragma once
#include <random>

#include "LinearGenerator.h"

namespace vox::rand
{

class GlobalPerlinField
{
private:
    uint64_t seed_;
    int period_;
    float amplitude_;
    LinearGenerator re_;
    std::uniform_real_distribution<float> dist_;

public:
    GlobalPerlinField(uint64_t seed, int period, float amplitude);
    float Sample(int x, int z);
};

}

