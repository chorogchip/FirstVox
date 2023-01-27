#pragma once

#include "Point2D.h"

namespace vox::rand::perlin
{

    class PerlinGeneratorUnit
    {
    private:
        const int level_;
        const float* const multipliers_;
        const vox::data::Pointf2D* const* point_vec_thetas_;
    public:
        PerlinGeneratorUnit(int x, int z, int level, const float* multipliers, unsigned seed = 0U);
        ~PerlinGeneratorUnit();
        float Sample( float x, float z ) const;
    };
}