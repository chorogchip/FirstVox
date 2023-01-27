#pragma once

#include "Macros.h"
#include "Consts.h"
#include "Vector.h"
#include "EnumSide.h"

namespace vox::gameutils
{
    FORCE_INLINE vox::data::Vector4i VEC_CALL GetChunkNumByBlockPos( vox::data::Vector4i pos )
    {
        static_assert(vox::consts::CHUNK_X_LOG2 == vox::consts::CHUNK_Z_LOG2);
        const auto sh_res = vox::data::vector::ShiftRightSignExtend( pos, vox::consts::CHUNK_X_LOG2 );
        const auto mask = vox::data::vector::Set( -1, 0, -1, 0 );
        return vox::data::vector::And( sh_res, mask );
    }

    FORCE_INLINE vox::data::Vector4i VEC_CALL GetBlockRemPosByPos( vox::data::Vector4i pos )
    {
        const auto mask = vox::data::vector::Set( vox::consts::CHUNK_X - 1, -1, vox::consts::CHUNK_Z - 1, 0 );
        return vox::data::vector::And( pos, mask );
    }

    // returns -1 if failed, returns 6 if ray is in hit block
    vox::data::EnumSideCollideResult VEC_CALL GetRayFirstCollidingBlockPos(
        vox::data::Vector4f ray_origin, vox::data::Vector4f ray_rotation,
        vox::data::Vector4i* res_pos
    );
}