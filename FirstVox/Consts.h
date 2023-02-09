#pragma once

#include <chrono>

#include "Utils.h"

namespace vox::consts
{
#if defined (DEBUG) || defined (_DEBUG)
    constexpr inline int DEBUG_ENABLED = 1;
#else
    constexpr inline int DEBUG_ENABLED = 0;
#endif

    constexpr inline float PI = 3.1415926535f;
    constexpr inline float PI_2 = PI * 2.0f;
    constexpr inline float PI_DIV2 = PI * 0.5f;
    constexpr inline float PI_DIV4 = PI * 0.25f;

    constexpr inline int MAX_FRAME_SKIP = 10;
    constexpr inline int TPS = 60;
    constexpr inline auto MICROSEC_PER_TICK = std::chrono::microseconds(1000LL * 1000LL / TPS);

    constexpr inline int MAP_Y = 256;
    constexpr inline int CHUNK_X = 32;
    constexpr inline int CHUNK_Z = 32;
    constexpr inline int CHUNK_Y = 256;
    constexpr inline int CHUNK_X_LOG2 = 32 - (int)std::countl_zero( (unsigned)vox::consts::CHUNK_X - 1U );
    constexpr inline int CHUNK_Y_LOG2 = 32 - (int)std::countl_zero( (unsigned)vox::consts::CHUNK_Y - 1U );
    constexpr inline int CHUNK_Z_LOG2 = 32 - (int)std::countl_zero( (unsigned)vox::consts::CHUNK_Z - 1U );
    constexpr inline int CHUNK_BLOCKS_CNT = CHUNK_Y * CHUNK_Z * CHUNK_X;
    
    static_assert(vox::utils::IsPowOf2( MAP_Y ));
    static_assert(vox::utils::IsPowOf2( CHUNK_X ));
    static_assert(vox::utils::IsPowOf2( CHUNK_Z ));
    static_assert(vox::utils::IsPowOf2( CHUNK_Y ));
    static_assert(MAP_Y % CHUNK_Y == 0);

    constexpr inline float CAM_SPEED = 32.0f;
    constexpr inline int INIT_RENDER_DIST = 8;
    constexpr inline int INIT_LOAD_DIST = INIT_RENDER_DIST + 1;
    constexpr inline int MAX_RENDER_DIST = 15;

    constexpr inline float NEAR_Z = 0.01f;
    constexpr inline float FAR_Z = (float)((vox::consts::MAX_RENDER_DIST + 3) * vox::consts::CHUNK_X);

    constexpr inline int TICKS_PER_DAY = 3600;
    constexpr inline int DAYS_PER_YEAR = 360;
    constexpr inline float LATITUDE = PI_DIV2 * 37.0f / 90.0f;
    constexpr inline float EARTH_TILT = PI_DIV2 * 23.5f / 90.0f;
    constexpr inline float SUN_ALTITUDE = FAR_Z * 0.9f;

    constexpr inline float TEX_BLOCK_WID_PIX = 256.0f;
    constexpr inline float TEX_BLOCK_HEI_PIX = 256.0f;
    constexpr inline float BLOCK_WID_PIX = 16.0f;
    constexpr inline float BLOCK_HEI_PIX = 16.0f;

    constexpr inline uint64_t GAMESTAT_MOUSEENABLED = 0x1ULL;
}