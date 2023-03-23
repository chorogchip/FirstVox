#pragma once

#include "FirstVoxHeader.h"
#include "Macros.h"
#include "Vector.h"

namespace vox::data::shapes
{
    // normal uvw, d(ux + vy + wz + d = 0)
    using Plane = Vector4f;
    // position xyz, radius r
    using Sphere = Vector4f;

    struct AABB
    {
        Vector4f position_min;
        Vector4f position_max;
    };
}

