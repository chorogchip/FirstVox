#pragma once
#include <random>

#include "LinearGenerator.h"
namespace vox::rand
{

    class GlobalUniformField
    {
    private:
        uint64_t seed_;
        LinearGenerator re_;
        std::uniform_real_distribution<float> dist_;

    public:
        GlobalUniformField(uint64_t seed);
        float Sample(int x, int z);
    };

}

