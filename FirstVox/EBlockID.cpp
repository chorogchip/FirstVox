#include "EBlockID.h"

#include "Consts.h"

namespace vox::data
{
    bool IsFullBlock( EBlockID block_id )
    {
        switch ( block_id )
        {
#define BL_ATR(name, is_full_block, ...) case EBlockID::name: return is_full_block;
            BLOCK_ATTRIB_TUPLE
#undef BL_ATR
        default: return false;
        }
    }

    static constexpr unsigned Texture_Pos_[] = {
#define BL_ATR(name, is_full_block, px, py, ...)\
((py << 8U) | (px & 0xf)),
    BLOCK_ATTRIB_TUPLE
#undef BL_ATR
    };
    const unsigned GetTexturePos( EBlockID block_id )
    {
        return Texture_Pos_[(int)block_id];
    }

    /*
    unsigned int GetRGB(EBlockID block_id)
    {
        switch ( block_id )
        {
#define BL_ATR(name, is_full_block, px, py, r, g, b, ...) case EBlockID::name: return r << 24 | g << 16 | b << 8;
            BLOCK_ATTRIB_TUPLE
#undef BL_ATR
        default: return 0;
        }
    }
    */
}