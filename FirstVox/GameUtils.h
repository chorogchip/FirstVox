#pragma once

#include "Macros.h"
#include "Consts.h"
#include "Vector.h"
#include "EnumSide.h"

namespace vox::gameutils
{

    // returns -1 if failed, returns 6 if ray is in hit block
    vox::data::EnumSideCollideResult VEC_CALL GetRayFirstCollidingBlockPos(
        vox::data::Vector4f ray_origin, vox::data::Vector4f ray_rotation,
        vox::data::Vector4i* res_pos
    );

    vox::data::Vector4f GetSkyColorBySunAltitude( float altitude );
}