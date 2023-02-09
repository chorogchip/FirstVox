#include "Vector.h"

namespace vox::data {

    std::ostream& operator<<( std::ostream& ostr, Vector4f v )
    {
        alignas(16) float f4[4];
        vox::data::vector::Store( f4, v );
        ostr << '[' << f4[0] << ' ' << f4[1] << ' ' << f4[2] << ' ' << f4[3] << ']';
        return ostr;
    }
    std::ostream& operator<<( std::ostream& ostr, Vector4i v )
    {
        alignas(16) int i4[4];
        vox::data::vector::Store( (Vector4i*)i4, v );
        ostr << '[' << i4[0] << ' ' << i4[1] << ' ' << i4[2] << ' ' << i4[3] << ']';
        return ostr;
    }

}
