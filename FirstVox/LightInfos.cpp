#include "LightInfos.h"

namespace vox::data::lightinfos
{
#define A(x) {x, x, x},
#define B(x, r, g, b) {((x)*(r)), ((x)*(g)), ((x)*(b))},//A(x) A(x+1) A(x+2) A(x+3)
#define C(x, r, g, b) B(x, r, g, b) B(x+4, r, g, b) B(x+8, r, g, b) B(x+12, r, g, b)
#define D(x, r, g, b) C(x, r, g, b) C(x+16, r, g, b) C(x+32, r, g, b) C(x+48, r, g, b)
#define E(x, r, g, b) D(x, r, g, b) D(x+64, r, g, b) D(x+128, r, g, b) D(x+192, r, g, b)
    static constexpr R8G8B8 TABLE_TEMP[] = { E( 3, 1, 1, 1 ) };
    static constexpr R8G8B8 TABLE_RED[] = { E( 3, 1, 0, 0 ) };
    static constexpr R8G8B8 TABLE_GREEN[] = { E( 3, 0, 1, 0 ) };
    static constexpr R8G8B8 TABLE_BLUE[] = { E( 3, 0, 0, 1 ) };
    static constexpr R8G8B8 TABLE_YELLOW[] = { E( 3, 1, 1, 0 ) };
    static constexpr R8G8B8 TABLE_CYAN[] = { E( 3, 0, 1, 1 ) };
    static constexpr R8G8B8 TABLE_MAGENTA[] = { E( 3, 1, 0, 1 ) };

#undef E
#undef D
#undef C
#undef B
#undef A
    const R8G8B8* GetRGBTable( EnumLightType type )
    {
        switch ( type )
        {
        case EnumLightType::POINT_BASIC:
        default:
            return TABLE_TEMP;
        case EnumLightType::POINT_RED:
            return TABLE_RED;
        case EnumLightType::POINT_GREEN:
            return TABLE_GREEN;
        case EnumLightType::POINT_BLUE:
            return TABLE_BLUE;
        case EnumLightType::POINT_YELLOW:
            return TABLE_YELLOW;
        case EnumLightType::POINT_CYAN:
            return TABLE_CYAN;
        case EnumLightType::POINT_MAGENTA:
            return TABLE_MAGENTA;
        }
    }
}