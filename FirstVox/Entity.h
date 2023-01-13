#pragma once

#include "Vector.h"

namespace vox::data
{
    struct Entity
    {
    public:
        Vector3 position;
        Vector3 speed;
        Vector3 rotation;
    };
}