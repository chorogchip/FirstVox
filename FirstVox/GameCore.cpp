#include "GameCore.h"

#include <cmath>

#include "Macros.h"
#include "Consts.h"
#include "ConstsTime.h"

#include "ChunkManager.h"

namespace vox::core::gamecore
{
    static uint32_t game_ticks = 0LL;
    static vox::data::Vector4f sun_vec;

    uint32_t GetGameTicks()
    {
        return game_ticks;
    }

    static FORCE_INLINE void CalcSunVec()
    {
        const int days = (int)(game_ticks / (uint32_t)vox::consts::TICKS_PER_DAY);
        const int rem_day_ticks = (int)(game_ticks % (uint32_t)vox::consts::TICKS_PER_DAY);

        const int years = (int)(game_ticks / (uint32_t)(vox::consts::TICKS_PER_DAY * vox::consts::DAYS_PER_YEAR));
        const int rem_year_ticks = (int)(game_ticks % (uint32_t)(vox::consts::TICKS_PER_DAY * vox::consts::DAYS_PER_YEAR));

        const float delta_day = vox::consts::PI_2 * (float)rem_day_ticks / (float)vox::consts::TICKS_PER_DAY;
        const float delta_year = vox::consts::PI_2 * (float)rem_year_ticks / (float)(vox::consts::TICKS_PER_DAY * vox::consts::DAYS_PER_YEAR);

        float e_pos[3], s_pos[3], up_pos[3], ex_pos[3];
        e_pos[0] = std::cosf( vox::consts::LATITUDE ) * std::cosf( delta_day );
        e_pos[1] = std::sinf( vox::consts::LATITUDE );
        e_pos[2] = std::sinf( delta_day + delta_year );

        const float sin_tilt = std::sinf( -vox::consts::EARTH_TILT );
        const float cos_tilt = std::cosf( -vox::consts::EARTH_TILT );
        const float e_pos_x = e_pos[0];
        const float e_pos_y = e_pos[1];
        e_pos[0] = e_pos_x * cos_tilt - e_pos_y * sin_tilt;
        e_pos[1] = e_pos_x * sin_tilt + e_pos_y * cos_tilt;

        s_pos[0] = std::cosf( delta_year );
        s_pos[1] = 0.0f;
        s_pos[2] = std::sinf( delta_year );

        up_pos[0] = 0.0f;
        up_pos[1] = 1.0f;
        up_pos[2] = 0.0f;

        vox::utils::cross3( ex_pos, up_pos, e_pos );
        vox::utils::cross3( up_pos, ex_pos, e_pos );

        sun_vec = vox::data::vector::Set(
            vox::utils::dot3( s_pos, ex_pos ),
            vox::utils::dot3( s_pos, e_pos ),
            vox::utils::dot3( s_pos, up_pos ),
            0.0f
        );
    }

    vox::data::Vector4f GetSunVec()
    {
        return sun_vec;
    }

    void Init()
    {
        auto start_pos = data::vector::Set(0, 0, 0, 0);
        data::EBlockID blk;
        while (blk = chunkmanager::GetBlock(start_pos).id,
            blk != data::EBlockID::AIR && blk != data::EBlockID::MAX_COUNT)
        {
            start_pos.m128i_i32[1]++;
        }
        start_pos.m128i_i32[1] += 2;
        if (start_pos.m128i_i32[1] == 2)
            start_pos.m128i_i32[1] = 35;

        camera.entity.SetPosition( data::vector::ConvertToVector4f( start_pos ));
        //camera.entity.SetSpeed( vox::data::vector::Set( 0.0f, 0.0f, 0.0f, 0.0f ) );
        for (int i = 0; i < 10; ++i)
            hand_blocks[i] = (vox::data::EBlockID)(i + 1);
    }

    void Update()
    {
        camera.entity.SetPosition( vox::data::vector::Add( camera.entity.GetPositionVec(), camera.entity.GetSpeedVec() ) );
        camera.entity.SetSpeed( vox::data::vector::SetZero4f() );

        CalcSunVec();

        ++game_ticks;
    }

    void Clean()
    {

    }
}