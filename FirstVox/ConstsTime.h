#pragma once
#include "Consts.h"

namespace vox::consts
{

    constexpr inline int TICKS_PER_DAY = 3600;
    constexpr inline int DAYS_PER_YEAR = 360;
    constexpr inline float LATITUDE = PI_DIV2 * 37.0f / 90.0f;
    constexpr inline float EARTH_TILT = PI_DIV2 * 23.5f / 90.0f;
    constexpr inline float SUN_ALTITUDE = ((float)((vox::consts::MAX_RENDER_DIST + 2) * vox::consts::CHUNK_X)) * 0.9f;

}