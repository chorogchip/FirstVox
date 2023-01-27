#include "EBlockID.h"

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

    static constexpr vox::data::Pointf2D Texture_Pos_[] = {
#define BL_ATR(name, is_full_block, px, py, ...)\
vox::data::Pointf2D{ (float)px * (16.0f / 256.0f), (float)py * (16.0f / 256.0f) },
    BLOCK_ATTRIB_TUPLE
#undef BL_ATR
    };
    const vox::data::Pointf2D& GetTexturePos( EBlockID block_id )
    {
        return Texture_Pos_[(int)block_id];
    }
}