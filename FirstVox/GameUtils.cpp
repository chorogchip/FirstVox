#include "GameUtils.h"

#include <cmath>
#include <climits>

#include "Logger.h"
#include "ChunkManager.h"

namespace vox::gameutils
{
    // returns -1 if failed, returns 6 if ray is in hit block
    vox::data::EnumSideCollideResult VEC_CALL GetRayFirstCollidingBlockPos(
        vox::data::Vector4f ray_origin, vox::data::Vector4f ray_rotation,
        vox::data::Vector4i* res_pos
    )
    {
        const auto round_pos = vox::data::vector::Round( ray_origin );
        alignas(16) int pos[4];
        vox::data::vector::Storeu( pos,
            vox::data::vector::ConvertToVector4i( round_pos ) );
        
        alignas(16) float ray_rot[4];
        vox::data::vector::Store( ray_rot, ray_rotation );

        const float costh = std::cos( ray_rot[0] );
        alignas(16) const float tlength[4] = {
            -costh * std::sin( ray_rot[1] ),
            std::sin( ray_rot[0] ),
            costh * std::cos( ray_rot[1] ),
            0.0f
        };

        const auto len_v = vox::data::vector::Load( tlength );
        alignas(16) float tdelta[4];
        const auto mask_v = vox::data::vector::Set1sIfEqual( len_v, vox::data::vector::SetZero4f() );
        const auto lenres_v = vox::data::vector::Add( len_v,
            vox::data::vector::And( mask_v, vox::data::vector::SetBroadcast(
                std::numeric_limits<float>::min()
            ) ) );
        const auto tdelta_v = vox::data::vector::Abs(
            vox::data::vector::ReciprocalApproximate( lenres_v ) );
        vox::data::vector::Store( tdelta, tdelta_v );

        const int sign_bit = vox::data::vector::GetSignBits( len_v );
        alignas(16) const int step[4] = {
            1 - ((sign_bit & 1) << 1),
            1 - ((sign_bit & 2)),
            1 - ((sign_bit & 4) >> 1),
            0
        };

        alignas(16) float tmax[4];
        const auto rem_len_v = vox::data::vector::Sub(
            vox::data::vector::SetBroadcast( 0.5f ),
            vox::data::vector::Sub( ray_origin, round_pos )
        );
        const auto step_v = vox::data::vector::CastToVector4f( vox::data::vector::Load( (vox::data::Vector4i*)step ) );
        const auto mask_real_rem_len_v_ = vox::data::vector::And( step_v, vox::data::vector::CastToVector4f( vox::data::vector::SetBroadcast( 0x80000000 ) ) );
        const auto addd_real_rem_len_v_ = vox::data::vector::And( step_v, vox::data::vector::SetBroadcast( 1.0f ) );
        const auto real_rem_len_v = vox::data::vector::Add(
            vox::data::vector::Xor( rem_len_v, mask_real_rem_len_v_),
            addd_real_rem_len_v_
        );
        const auto tmax_v = vox::data::vector::Mul( tdelta_v, real_rem_len_v );
        vox::data::vector::Store( tmax, tmax_v );

        constexpr int MAX_RAY_T = 100;
        vox::data::EnumSideCollideResult ret_side_arr[4] = {
            (vox::data::EnumSideCollideResult)((int)vox::data::EnumSideCollideResult::LEFT - ((sign_bit & 1) >> 0)),
            (vox::data::EnumSideCollideResult)((int)vox::data::EnumSideCollideResult::DOWN - ((sign_bit & 2) >> 1)),
            (vox::data::EnumSideCollideResult)((int)vox::data::EnumSideCollideResult::BACK - ((sign_bit & 4) >> 2)),
            vox::data::EnumSideCollideResult::INSIDE,
        };

        int index = 3;
        for ( int i = 0; i < MAX_RAY_T; ++i )
        {
            if ( pos[1] >= 0 && pos[1] < vox::consts::MAP_Y )
            {
                const auto bpv = vox::data::vector::Load( (vox::data::Vector4i*)pos );
                const auto block = vox::core::chunkmanager::GetBlock( bpv );
                if ( block.id == vox::data::EBlockID::MAX_COUNT ) break;
                if ( block.id != vox::data::EBlockID::AIR )
                {
                    vox::data::vector::Store( res_pos, bpv );
                    return ret_side_arr[index];
                }
            }

            if ( tmax[0] < tmax[1] )
            {
                if ( tmax[0] < tmax[2] ) index = 0;
                else index = 2;
            }
            else
            {
                if ( tmax[1] < tmax[2] ) index = 1;
                else index = 2;
            }

            pos[index] += step[index];
            tmax[index] += tdelta[index];
        }

        return vox::data::EnumSideCollideResult::FAILED;
    }

    static const vox::data::Vector4f colors[] = { 
        vox::data::vector::SetByHexColor(0x0059ff),
        vox::data::vector::SetByHexColor(0x3bb0ff),
        vox::data::vector::SetByHexColor(0x3bb0ff),
        vox::data::vector::SetByHexColor(0x70d4ff),
        vox::data::vector::SetByHexColor(0xff8400),
        vox::data::vector::SetByHexColor(0x1c0040),
        vox::data::vector::SetByHexColor(0x010017),
        vox::data::vector::SetByHexColor(0x010017),
        vox::data::vector::SetByHexColor(0x010017),
    };

    vox::data::Vector4f GetSkyColorBySunAltitude( float altitude )
    {
        const float index_f = (1.0f - altitude) * 4.0f;
        int index = (int)index_f;
        if ( index < 0 ) index = 0;
        else if ( index > 7 ) index = 7;
        const float rem = index_f - (float)index;
        return vox::data::vector::Add(
            vox::data::vector::Mul( colors[index], vox::data::vector::SetBroadcast( 1.0f - rem ) ),
            vox::data::vector::Mul( colors[index + 1], vox::data::vector::SetBroadcast( rem ) )
        );
    }
}
