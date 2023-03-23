#include "Camera.h"

#include "Consts.h"
#include "RendererViewer.h"

namespace vox::core::camera
{
    Camera::Camera() :
        fov_y_( vox::consts::PI_DIV4 ),
        asepct_ratio_( 0.0f ),
        entity {}
    {
    }



    // order : right, left, up, down, far, near
    void Camera::GenerateViewFrustum( vox::data::shapes::Plane plane_dest[6] ) const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;

        const auto front = entity.GetForwardVec();
        const auto right = entity.GetRightVec();
        const auto up = entity.GetUpVec();
        const auto campos = entity.GetPositionVec();

        // near d = -projN(cam + N * nearZ) = -N dot cam - nearZ

        const float d = -vox::data::vector::Dot3ToScalar( &front, &campos );
        plane_dest[5] = front;
        plane_dest[4] = vox::data::vector::Minus( front );
        ((float*)&plane_dest[5])[3] = d - GetNearZ();
        ((float*)&plane_dest[4])[3] = GetFarZ() - d;

        const float half_height = std::tanf( GetFovY() * 0.5f );
        const float half_width = half_height * GetAspectRatio();

        const auto half_width_vec = vox::data::vector::Mul( right, vox::data::vector::SetBroadcast( half_width ) );
        const auto half_height_vec = vox::data::vector::Mul( up, vox::data::vector::SetBroadcast( half_height ) );

        const auto to_right_vec = vox::data::vector::Add( front, half_width_vec );
        const auto to_left_vec = vox::data::vector::Sub( front, half_width_vec );
        const auto to_up_vec = vox::data::vector::Add( front, half_height_vec );
        const auto to_down_vec = vox::data::vector::Sub( front, half_height_vec );

        vox::utils::cross3(
            (float*)&plane_dest[0],
            (const float*)&to_right_vec,
            (const float*)&up );
        vox::utils::cross3(
            (float*)&plane_dest[1],
            (const float*)&up,
            (const float*)&to_left_vec );
        vox::utils::cross3(
            (float*)&plane_dest[2],
            (const float*)&right,
            (const float*)&to_up_vec );
        vox::utils::cross3(
            (float*)&plane_dest[3],
            (const float*)&to_down_vec,
            (const float*)&right );
        
        vox::utils::normalize3( (float*)&plane_dest[0] );
        vox::utils::normalize3( (float*)&plane_dest[1] );
        vox::utils::normalize3( (float*)&plane_dest[2] );
        vox::utils::normalize3( (float*)&plane_dest[3] );

        ((float*)&plane_dest[0])[3] = -vox::data::vector::Dot3ToScalar( &plane_dest[0], &campos );
        ((float*)&plane_dest[1])[3] = -vox::data::vector::Dot3ToScalar( &plane_dest[1], &campos );
        ((float*)&plane_dest[2])[3] = -vox::data::vector::Dot3ToScalar( &plane_dest[2], &campos );
        ((float*)&plane_dest[3])[3] = -vox::data::vector::Dot3ToScalar( &plane_dest[3], &campos );
        
    }
}