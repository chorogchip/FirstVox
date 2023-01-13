#pragma once

namespace vox::utils
{
    constexpr bool IsPowOf2( int x )
    {
        return ((x - 1) & x) == 0;
    }
    
    consteval float Sqrtf(float d)
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
}