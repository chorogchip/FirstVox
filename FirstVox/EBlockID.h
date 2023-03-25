#pragma once
#include "Point2D.h"

namespace vox::data {

#define BLOCK_ATTRIB_TUPLE \
/*          name, is_full_block, texture_pos_x,y */\
BL_ATR(AIR,        false, 0, 0)\
BL_ATR(GRASS,       true, 3, 1)\
BL_ATR(DIRT,        true, 3, 0)\
BL_ATR(SAND,        true, 4, 0)\
BL_ATR(STONE,       true, 1, 0)\
BL_ATR(COBBLESTONE, true, 2, 0)\
BL_ATR(DIAMOND_ORE, true, 7, 1)\
BL_ATR(TREE_Y,      true, 6, 0)\
BL_ATR(TREE_Z,      true, 5, 0)\
BL_ATR(TREE_X,      true, 5, 0)\
BL_ATR(LEAF,        true, 8, 0)\
BL_ATR(MAX_COUNT,  false, 0, 0)

    enum class EBlockID : unsigned short
    {
#define BL_ATR(name, ...) name, 
        BLOCK_ATTRIB_TUPLE
#undef BL_ATR
    };

    bool IsFullBlock( EBlockID block_id );
    const unsigned GetTexturePos( EBlockID block_id );

    // pervious EnumBlocks
    /*
    enum class EnumBlocks
    {
        AIR = 0,
        STONE = 1,
        COBBLESTONE = 2,
        GRASS = 3,
        DIRT = 4,
        SAND = 5,
        LOG = 6,
        LEAVES = 7,
        PLANK = 8,
        PLANK_HALF = 9,
        GLASS = 10,
        GLASS_PANE = 11,
        GLASS_STAINED = 12,
        GLASS_PANE_STAINED = 13,
        FERN = 14,
        GLOWSTONE = 15,
        WATER = 16,
        LAVA = 17,
    };
    */

}