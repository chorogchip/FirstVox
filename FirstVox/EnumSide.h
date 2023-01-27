#pragma once

#include "Macros.h"
#include "Vector.h"

namespace vox::data
{
    enum class EnumSide
    {
        UP = 0,
        DOWN = 1,
        FRONT = 2,
        BACK = 3,
        RIGHT = 4,
        LEFT = 5,

        MAX_COUNT = 6,
    };

    enum class EnumSideCollideResult
    {
        UP = 0,
        DOWN = 1,
        FRONT = 2,
        BACK = 3,
        RIGHT = 4,
        LEFT = 5,

        INSIDE = 6,
        FAILED = 7,
    };

    FORCE_INLINE EnumSide ToEnumSide( EnumSideCollideResult res )
    {
        return (EnumSide)res;
    }

    inline const vox::data::Vector4i DIRECTION4_V4I[4] = {
        vox::data::vector::Set( 0, 0, 1, 0 ),
        vox::data::vector::Set( 0, 0,-1, 0 ),
        vox::data::vector::Set( 1, 0, 0, 0 ),
        vox::data::vector::Set( -1, 0, 0, 0 ),
    };

    enum class EnumBitSide6
    {
        UP = 1 << (int)EnumSide::UP,
        DOWN = 1 << (int)EnumSide::DOWN,
        FRONT = 1 << (int)EnumSide::FRONT,
        BACK = 1 << (int)EnumSide::BACK,
        RIGHT = 1 << (int)EnumSide::RIGHT,
        LEFT = 1 << (int)EnumSide::LEFT,

        FULL_VALUE = ( 1 << (int)EnumSide::MAX_COUNT ) - 1,
    };

    FORCE_INLINE vox::data::Vector4i VEC_CALL EnumSideToVec4i( EnumSide side )
    {
        switch ( side )
        {
        case EnumSide::UP: return vox::data::vector::Set( 0, 1, 0, 0 );
        case EnumSide::DOWN: return vox::data::vector::Set( 0, -1, 0, 0 );
        case EnumSide::FRONT: return vox::data::vector::Set( 0, 0, 1, 0 );
        case EnumSide::BACK: return vox::data::vector::Set( 0, 0, -1, 0 );
        case EnumSide::RIGHT: return vox::data::vector::Set( 1, 0, 0, 0 );
        case EnumSide::LEFT: return vox::data::vector::Set( -1, 0, 0, 0 );
        default: return vox::data::vector::SetZero4i();
        }
    }
}