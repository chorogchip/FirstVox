#pragma once

#include "Consts.h"
#include "Entity.h"
#include "Shapes.h"


namespace vox::core::camera
{
    class Camera
    {
    private:
        float fov_y_;
        float asepct_ratio_;
    public:
        vox::data::Entity entity;

        Camera();

        float GetNearZ() const
        {
            return 0.01f;
        }
        float GetFarZ() const
        {
            return (float)((vox::consts::MAX_RENDER_DIST + 2) * vox::consts::CHUNK_X);
        }
        float GetFovY() const
        {
            return fov_y_;
        }
        float GetAspectRatio() const
        {
            return asepct_ratio_;
        }

        void SetAspectRatio( float aspect_ratio )
        {
            asepct_ratio_ = aspect_ratio;
        }
        // order : right, left, up, down, far, near
        void GenerateViewFrustum( vox::data::shapes::Plane plane_dest[6] ) const;
    };

}
