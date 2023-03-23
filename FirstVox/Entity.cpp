#include "Entity.h"

#include "Marker.h"
#include "Consts.h"

namespace vox::data
{
    // 화전 순서 : [RY][RX][RZ][V] (오일러 각, 반시계방향 회전, 왼손 좌표계)

    Vector4f Entity::GetEulerRotationVec() const
    {
        return rotation_;
    
    }
    float Entity::GetRotationX() const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        const float* rot_ptr = (const float*)&rotation_;
        return rot_ptr[0];
    }

    float Entity::GetRotationY() const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        const float* rot_ptr = (const float*)&rotation_;
        return rot_ptr[1];
    }

    float Entity::GetRotationZ() const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        const float* rot_ptr = (const float*)&rotation_;
        return rot_ptr[2];
    }

    Vector4f Entity::GetForwardVec() const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        const float* rot_ptr = (const float*)&rotation_;

        const float sx = std::sin( rot_ptr[0] );
        const float cx = std::cos( rot_ptr[0] );
        const float sy = std::sin( rot_ptr[1] );
        const float cy = std::cos( rot_ptr[1] );
        return vox::data::vector::Set(
            -cx*sy,
            sx,
            cx*cy,
            0.0f
        );
    }

    Vector4f Entity::GetUpVec() const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        const float* rot_ptr = (const float*)&rotation_;

        const float sx = std::sin( rot_ptr[0] );
        const float cx = std::cos( rot_ptr[0] );
        const float sy = std::sin( rot_ptr[1] );
        const float cy = std::cos( rot_ptr[1] );
        const float sz = std::sin( rot_ptr[2] );
        const float cz = std::cos( rot_ptr[2] );
        return vox::data::vector::Set(
            sz*cy + cz*sx*sy,
            cz*cx,
            sz*sy - cz*sx*cy,
            0.0f
        );
    }

    Vector4f Entity::GetRightVec() const
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        const float* rot_ptr = (const float*)&rotation_;

        const float sx = std::sin( rot_ptr[0] );
        const float cx = std::cos( rot_ptr[0] );
        const float sy = std::sin( rot_ptr[1] );
        const float cy = std::cos( rot_ptr[1] );
        const float sz = std::sin( rot_ptr[2] );
        const float cz = std::cos( rot_ptr[2] );
        return vox::data::vector::Set(
            cz*cy - sz*sx*sy,
            -sz*cx,
            cz*sy + sz*sx*cy,
            0.0f
        );
    }

    void Entity::RotateX( float radians )
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        float* rot_ptr = (float*)&rotation_;
        rot_ptr[0] = std::remainder( rot_ptr[0] + radians, vox::consts::PI_2 );
    }

    void Entity::RotateY( float radians )
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        float* rot_ptr = (float*)&rotation_;
        rot_ptr[1] = std::remainder( rot_ptr[1] + radians, vox::consts::PI_2 );
    }

    void Entity::RotateZ( float radians )
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        float* rot_ptr = (float*)&rotation_;
        rot_ptr[2] = std::remainder( rot_ptr[2] + radians, vox::consts::PI_2 );
    }
    // after rotate, clamp in [pi/2, pi/2]
    void Entity::RotateXClamp( float radians )
    {
        USING_SIMD_PTR_TO_PRIMITIVE_PTR_TRICK;
        float* rot_ptr = (float*)&rotation_;
        rot_ptr[0] = std::clamp( rot_ptr[0] + radians, -vox::consts::PI_DIV2, vox::consts::PI_DIV2 );
    }
}