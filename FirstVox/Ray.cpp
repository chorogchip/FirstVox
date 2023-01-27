#include "Ray.h"

namespace vox::data
{

    Ray::Ray( vox::data::Vector4f origin, vox::data::Vector4f dv ):
        origin_{origin},
        dv_{ dv },
        t_{ 0.0f }
    { }

}
