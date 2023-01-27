#include "Vector.h"

namespace vox::data {

    std::ostream& operator<<( std::ostream& ostr, const Vector4f& v )
    {
        float f4[4];
        v.ToFloat4( f4 );
        ostr << '[' << f4[0] << ' ' << f4[1] << ' ' << f4[2] << ' ' << f4[3] << ']';
        return ostr;
    }
    std::ostream& operator<<( std::ostream& ostr, const Vector4i& v )
    {
        int i4[4];
        v.ToInt4( i4 );
        ostr << '[' << i4[0] << ' ' << i4[1] << ' ' << i4[2] << ' ' << i4[3] << ']';
        return ostr;
    }

}
