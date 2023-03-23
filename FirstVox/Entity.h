#pragma once

#include "Macros.h"
#include "Vector.h"

namespace vox::data
{
    struct Entity
    {
    private:
        Vector4f position_;
        Vector4f speed_;
        Vector4f rotation_;
    public:
        Entity() = default;

        Vector4f FORCE_INLINE GetPositionVec() const
        {
            return position_;
        }
        Vector4f FORCE_INLINE GetSpeedVec() const
        {
            return speed_;
        }
        Vector4f GetEulerRotationVec() const;
        float GetRotationX() const;
        float GetRotationY() const;
        float GetRotationZ() const;
        Vector4f GetForwardVec() const;
        Vector4f GetUpVec() const;
        Vector4f GetRightVec() const;

        void FORCE_INLINE SetPosition( Vector4f position )
        {
            vox::data::vector::Store( (float*)&position_, position );
        }
        void FORCE_INLINE SetSpeed( Vector4f speed )
        {
            vox::data::vector::Store( (float*)&speed_, speed );
        }
        void RotateX( float radians );
        void RotateY( float radians );
        void RotateZ( float radians );
        // after rotate, clamp in [pi/2, pi/2]
        void RotateXClamp( float radians );
    };
}