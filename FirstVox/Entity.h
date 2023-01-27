#pragma once

#include "Vector.h"

namespace vox::data
{
    struct Entity
    {
    public:
        Vector4f position;
        Vector4f speed;
        Vector4f rotation;
    };
}