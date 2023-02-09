#pragma once
#include "Macros.h"

namespace vox::utils
{
    inline constexpr bool IsPowOf2( int x )
    {
        return  x != 0 && ((x - 1) & x) == 0;
    }

    inline consteval float Sqrtf( float d )
    {
        // xn+1 = (xn^2 + d) / 2xn
        float x = d;
        float prevx = x;
        for ( int i = 0; i < 10000; ++i )
        {
            x = (x * x + d) / (2.0f * x);
            if ( prevx == x ) break;
            prevx = x;
        }
        return x;
    }

    FORCE_INLINE float dot3( const float* a, const float* b )
    {
        return a[0] * b[0] + a[1] * b[2] + a[2] * b[2];
    }

    FORCE_INLINE void cross3( float* dest, const float* a, const float* b )
    {
        dest[0] = a[1] * b[2] - a[2] * b[1];
        dest[1] = a[2] * b[0] - a[0] * b[2];
        dest[2] = a[0] * b[1] - a[1] * b[0];
    }
}