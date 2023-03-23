#include "EventHandler.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>

#include "Consts.h"
#include "Utils.h"
#include "GameUtils.h"
#include "Vector.h"
#include "Logger.h"
#include "EnumSide.h"

#include "GameCore.h"
#include "ChunkManager.h"

namespace vox::core::eventhandler {

    void OnKeyPressed( unsigned short c ) {
        switch ( c )
        {
        case VK_ESCAPE:
            break;
        case 'Q':
            if ( vox::core::gamecore::hand_block != vox::data::EBlockID::AIR )
            {
                vox::core::gamecore::hand_block = vox::data::EBlockID::AIR;
            }
            else
            {
                vox::data::Vector4i bpv;
                const auto col_side = vox::gameutils::GetRayFirstCollidingBlockPos(
                    vox::core::gamecore::camera.entity.GetPositionVec(),
                    vox::core::gamecore::camera.entity.GetEulerRotationVec(),
                    &bpv
                );
                if ( col_side != vox::data::EnumSideCollideResult::FAILED )
                {
                    const auto blk_id = vox::core::chunkmanager::GetBlock( bpv ).id;
                    if ( blk_id != vox::data::EBlockID::MAX_COUNT )
                    {
                        vox::core::gamecore::hand_block = blk_id;
                    }
                }
            }
            break;
        case '1':
            vox::core::gamecore::hand_block = (vox::data::EBlockID)(
                ((int)vox::core::gamecore::hand_block + 1) %
                (int)vox::data::EBlockID::MAX_COUNT
                );
            break;
        case '2':
            vox::core::gamecore::hand_block = (vox::data::EBlockID)(
                ((int)vox::core::gamecore::hand_block + (int)vox::data::EBlockID::MAX_COUNT - 1) %
                (int)vox::data::EBlockID::MAX_COUNT
                );
            break;
        case '3':
            vox::core::gamecore::hand_block = (vox::data::EBlockID)(
                ((int)vox::core::gamecore::hand_block + 10) %
                (int)vox::data::EBlockID::MAX_COUNT
                );
            break;
        case '4':
            vox::core::gamecore::hand_block = (vox::data::EBlockID)(
                ((int)vox::core::gamecore::hand_block + (int)vox::data::EBlockID::MAX_COUNT - 10) %
                (int)vox::data::EBlockID::MAX_COUNT
                );
            break;
        case VK_OEM_PLUS:
            {
                const auto ren_dist = vox::core::chunkmanager::GetRenderChunkDist();
                vox::core::chunkmanager::CleanDynamicChunkLoader( &vox::core::gamecore::camera.entity );
                vox::core::chunkmanager::RegisterDynamicChunkLoader( &vox::core::gamecore::camera.entity, ren_dist + 3);
                vox::core::chunkmanager::SetRenderChunkDist( ren_dist + 1 );
            }
            break;
        case VK_OEM_MINUS:
            {
                const auto ren_dist = vox::core::chunkmanager::GetRenderChunkDist();
                vox::core::chunkmanager::CleanDynamicChunkLoader( &vox::core::gamecore::camera.entity );
                vox::core::chunkmanager::RegisterDynamicChunkLoader( &vox::core::gamecore::camera.entity, ren_dist + 1);
                vox::core::chunkmanager::SetRenderChunkDist( ren_dist - 1 );
            }
            break;
        }
    }
    void OnKeyReleased( unsigned short  c ) {

    }
    void OnChar( char c ) {

    }
    void OnMouseLPressed( short x, short y ) {
        vox::data::Vector4i bpv;
        const auto col_side = vox::gameutils::GetRayFirstCollidingBlockPos(
            vox::core::gamecore::camera.entity.GetPositionVec(),
            vox::core::gamecore::camera.entity.GetEulerRotationVec(),
            &bpv
        );
        if ( col_side != vox::data::EnumSideCollideResult::FAILED )
        {
            vox::core::chunkmanager::SetBlock( bpv, vox::data::Block( vox::data::EBlockID::AIR ) );
        }
    }
    void OnMouseLReleased( short x, short y ) {

    }
    void OnMouseMPressed( short x, short y ) {

    }
    void OnMouseMReleased( short x, short y ) {

    }
    void OnMouseRPressed( short x, short y ) {
        if ( vox::core::gamecore::hand_block != vox::data::EBlockID::AIR )
        {
            vox::data::Vector4i bpv;
            const auto col_side = vox::gameutils::GetRayFirstCollidingBlockPos(
                vox::core::gamecore::camera.entity.GetPositionVec(),
                vox::core::gamecore::camera.entity.GetEulerRotationVec(),
                &bpv
            );
            if ( col_side != vox::data::EnumSideCollideResult::FAILED && col_side != vox::data::EnumSideCollideResult::INSIDE )
            {
                const auto side = ToEnumSide( col_side );
                const auto dbpv = vox::data::EnumSideToVec4i( side );
                const auto fbpv = vox::data::vector::Add( bpv, dbpv );
                const auto y = vox::data::vector::GetY(fbpv);
                if (y >= 0 && y < vox::consts::MAP_Y)
                {
                    vox::core::chunkmanager::SetBlock( fbpv,
                        vox::data::Block(vox::core::gamecore::hand_block) );
                }
            }
        }
    }
    void OnMouseRReleased( short x, short y ) {

    }
    void OnMouseMoved( short x, short y ) {

    }
    void OnWheelScrolled( int delta ) {

    }

    void OnRawMouseInput( int dx, int dy )
    {
        vox::core::gamecore::camera.entity.RotateY( dx * -0.003f );
        vox::core::gamecore::camera.entity.RotateXClamp( dy * -0.003f );
    }

    void Update()
    {
        // this is premature optimization, I regret this

        unsigned int pressed_w = (unsigned short)GetKeyState( 'W' ) & 0x8000;
        unsigned int pressed_s = (unsigned short)GetKeyState( 'S' ) & 0x8000;
        unsigned int pressed_a = (unsigned short)GetKeyState( 'A' ) & 0x8000;
        unsigned int pressed_d = (unsigned short)GetKeyState( 'D' ) & 0x8000;
        unsigned int pressed_sh = (unsigned short)GetKeyState( VK_LSHIFT ) & 0x8000;
        unsigned int pressed_sp = (unsigned short)GetKeyState( VK_SPACE ) & 0x8000;
        unsigned int pressed_keys = pressed_w >> 15 | pressed_s >> 14 | pressed_a >> 11 | pressed_d >> 10
            | pressed_sh >> 7 | pressed_sp >> 6;

        if ( pressed_keys )
        {
            static unsigned int prev_pressed_keys = 0;
            unsigned int pressed_both = pressed_keys & (pressed_keys >> 1);
            pressed_both |= pressed_both << 1;
            pressed_keys -= pressed_both & prev_pressed_keys;
            prev_pressed_keys = pressed_both ^ pressed_keys;

            constexpr float SPD = vox::consts::CAM_SPEED / (float)vox::consts::TPS;

            alignas(16) float spd[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            if ( int pressed_keys_y = pressed_keys >> 8 )
            {
                pressed_keys_y = ((pressed_keys_y ^ 1) - 1) >> 1;
                spd[1] = (float)pressed_keys_y * SPD;
            }
            if ( pressed_keys & 63 )
            {
                pressed_keys = (pressed_keys | pressed_keys >> 2) & 15;
                // 0 2 6 0 4 3 5 4 0 1 7 0 0 2 6 0 starting from lsb, 4 bit each
                int theta_cnt = (int)(0x0620071045340620LL >> (uint64_t)(pressed_keys * 4U)) & 15;
                float yaw = vox::core::gamecore::camera.entity.GetRotationY();
                float theta = yaw + (float)theta_cnt * vox::consts::PI_DIV4;
                spd[0] = std::cosf( theta ) * SPD;
                spd[2] = std::sinf( theta ) * SPD;
            }
            vox::core::gamecore::camera.entity.SetSpeed( vox::data::vector::Load( spd ) );
        }
    }
}