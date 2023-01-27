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

    constexpr inline float CAM_SPEED = 16.0f;
    constexpr inline int MAX_RENDER_DIST = 15;

    constexpr inline uint64_t GAMESTAT_MOUSEENABLED = 0x1ULL;
}