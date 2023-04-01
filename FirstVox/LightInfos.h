#pragma once
#include <vector>

namespace vox::data::lightinfos
{
    enum class EnumLightType
    {
        POINT_BASIC,
        POINT_RED,
        POINT_GREEN,
        POINT_BLUE,
        POINT_YELLOW,
        POINT_CYAN,
        POINT_MAGENTA,

        MAX_SIZE,
    };

    struct LightInstance
    {
        unsigned positions_MSB_X6_Z8_Y8_LSB;
        unsigned power;
    };
    struct LightTypesInfo
    {
        std::vector<LightInstance> instances;
        EnumLightType type;
    };

    struct R8G8B8
    {
        unsigned char R;
        unsigned char G;
        unsigned char B;
    };

    const R8G8B8* GetRGBTable( EnumLightType type );
}
