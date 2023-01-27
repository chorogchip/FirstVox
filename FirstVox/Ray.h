#pragma once

#include "Vector.h"

namespace vox::data
{
    class Ray
    {
    private:
        vox::data::Vector4f origin_;
        vox::data::Vector4f dv_;
        float t_;/*
        float tmax_x_, tmax_y_, tmax_z_;
        const float tdelta_x_, tdelta_y_, tdelta_z_;
        const int vox_x_, vox_y_, vox_z_;
        const int step_x_, step_y_, step_z_;*/

    public:
        Ray( vox::data::Vector4f origin, vox::data::Vector4f dv );
    };

}

