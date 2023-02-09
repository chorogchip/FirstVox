#include "GameCore.h"

#include <cmath>

#include "Consts.h"

namespace vox::core::gamecore
{
    static uint32_t game_ticks = 0LL;

    uint32_t GetGameTicks()
    {
        return game_ticks;
    }

    vox::data::Vector4f GetSunVec()
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

        return vox::data::vector::Set(
            vox::utils::dot3( s_pos, ex_pos ),
            vox::utils::dot3( s_pos, e_pos ),
            vox::utils::dot3( s_pos, up_pos ),
            0.0f
        );
    }

    void Init()
    {
        camera.position = vox::data::Vector4f{ 0.0f, 40.0f, -6.0f, 0.0f };
        camera.speed = vox::data::Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f };
        camera.rotation = vox::data::Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    void Update()
    {
        camera.position = vox::data::vector::Add( camera.position, camera.speed );
        camera.speed = vox::data::vector::SetZero4f();

        ++game_ticks;
    }

    void Clean()
    {

    }
}