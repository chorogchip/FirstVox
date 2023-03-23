#include "Camera.h"

#include "Consts.h"

namespace vox::core::camera
{
    Camera::Camera() :
        fov_y_( vox::consts::PI_DIV4 ),
        entity {}
    {
    }

    // order : right, left, up, down, far, near
    void Camera::GenerateViewFrustum( vox::data::shapes::Plane plane_dest[6] ) const
    {

    }
}